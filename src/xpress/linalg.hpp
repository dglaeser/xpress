// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup LinearAlgebra
 * \brief Algebraic operations on types that represent tensors or vectors.
 */
#pragma once

#include <concepts>
#include <type_traits>
#include <algorithm>

#include "utils.hpp"
#include "traits.hpp"
#include "operators.hpp"


namespace xp::linalg {

// forward declaration
namespace traits { template<typename T> struct access; }


//! \addtogroup LinearAlgebra
//! \{

template<typename T, typename shape>
struct tensor {
 public:
    constexpr tensor() = default;
    constexpr tensor(T value) noexcept { std::ranges::fill(_values, value); }

    template<std::convertible_to<T>... _T> requires(sizeof...(_T) == shape::count)
    constexpr tensor(const shape&, _T&&... values) noexcept
    : _values{static_cast<T>(std::forward<_T>(values))...}
    {}

    constexpr tensor(const shape&, std::array<T, shape::count>&& values) noexcept
    : _values{std::move(values)}
    {}

    template<typename S, std::size_t... i>
    constexpr decltype(auto) operator[](this S&& self, const md_index<i...>&) noexcept {
        static_assert(md_index<i...>::as_flat_index_in(shape{}) < shape::count);
        return self._values[md_index<i...>::as_flat_index_in(shape{})];
    }

    template<typename S, std::size_t i> requires(shape::dimensions == 1)
    constexpr decltype(auto) operator[](this S&& self, const index_constant<i>&) noexcept {
        return self[md_ic<i>];
    }

    template<typename V> requires(xp::traits::is_scalar_v<V>)
    constexpr auto operator*(const V& value) const noexcept {
        return operators::traits::multiplication_of<tensor, V>{}(*this, value);
    }

    template<typename T2, typename shape2>
    constexpr bool operator==(const tensor<T2, shape2>& other) const noexcept {
        if constexpr (shape2{} != shape{}) {
            return false;
        } else {
            bool all_equal = true;
            visit_indices_in(shape{}, [&] (const auto& idx) constexpr {
                if ((*this)[idx] != other[idx])
                    all_equal = false;
            });
            return all_equal;
        }
    }

    template<typename _T> requires(!std::is_const_v<T> and is_complete_v<traits::access<_T>>)
    constexpr void export_to(_T& out) const noexcept {
        visit_indices_in(shape{}, [&] (const auto& idx) constexpr {
            traits::access<_T>::at(idx, out) = (*this)[idx];
        });
    }

 private:
    std::array<T, shape::count> _values;
};

template<std::size_t... s, typename T, std::size_t size>
    requires(md_shape<s...>::count == size)
tensor(const md_shape<s...>&, std::array<T, size>&&) -> tensor<T, md_shape<s...>>;

template<std::size_t... s, typename... Ts>
    requires(std::conjunction_v<xp::traits::is_scalar<std::remove_cvref_t<Ts>>...>)
tensor(const md_shape<s...>&, Ts&&...) -> tensor<std::common_type_t<Ts...>, md_shape<s...>>;

template<std::size_t... s, typename... Ts>
    requires(sizeof...(Ts) > 0 and !std::conjunction_v<xp::traits::is_scalar<std::remove_cvref_t<Ts>>...>)
tensor(const md_shape<s...>&, Ts&&...) -> tensor<std::remove_cvref_t<first_t<type_list<Ts...>>>, md_shape<s...>>;

namespace traits {

#ifndef DOXYGEN
namespace detail {

    template<std::size_t s>
    void _invoke_with_constexpr_size() {}

    template<typename T>
    concept has_constexpr_size = requires(const T& t) {
        { t.size() } -> std::convertible_to<std::size_t>;
        { _invoke_with_constexpr_size<T{}.size()>() };
    };

    template<typename T>
    concept has_constexpr_size_constant = requires {
        { T::size } -> std::convertible_to<std::size_t>;
    };

}  // namespace detail
#endif  // DOXYGEN

template<typename T>
struct size_of;
template<detail::has_constexpr_size T>
struct size_of<T> : std::integral_constant<std::size_t, T{}.size()> {};
template<detail::has_constexpr_size_constant T>
struct size_of<T> : std::integral_constant<std::size_t, T::size> {};
template<typename T> requires(is_complete_v<size_of<T>>)
inline constexpr std::size_t size_of_v = size_of<T>::value;


#ifndef DOXYGEN
namespace detail {

    template<typename T, std::size_t... s>
    struct shape_of_indexable;
    template<typename T, std::size_t... s> requires(!xp::traits::is_indexable_v<T>)
    struct shape_of_indexable<T, s...> : std::type_identity<md_shape<s...>> {};
    template<typename T, std::size_t... s> requires(xp::traits::is_indexable_v<T>)
    struct shape_of_indexable<T, s...> : shape_of_indexable<xp::traits::value_type_t<T>, s..., size_of_v<T>> {};

}  // namespace detail
#endif  // DOXYGEN

template<typename T>
struct shape_of;
template<typename T> requires(xp::traits::is_indexable_v<T>)
struct shape_of<T> : detail::shape_of_indexable<T> {};
template<typename T, typename shape>
struct shape_of<tensor<T, shape>> : std::type_identity<shape> {};
template<typename T>
using shape_of_t = typename shape_of<T>::type;

template<typename T>
struct access;
template<typename T, typename shape>
struct access<tensor<T, shape>> {
    template<same_remove_cvref_t_as<tensor<T, shape>> _T, std::size_t... i>
    static constexpr decltype(auto) at(const md_index<i...>& idx, _T&& tensor) noexcept {
        return tensor[idx];
    }
};
template<typename T> requires(xp::traits::is_indexable_v<T> and is_complete_v<shape_of<T>>)
struct access<T> {
    template<same_remove_cvref_t_as<T> _T, std::size_t... i> requires(sizeof...(i) == shape_of_t<T>::dimensions)
    static constexpr decltype(auto) at(const md_index<i...>&, _T&& tensor) noexcept {
        return _at<i...>(std::forward<_T>(tensor));
    }

 private:
    template<std::size_t i, std::size_t... is>
    static constexpr decltype(auto) _at(auto&& t) noexcept {
        if constexpr (sizeof...(is) == 0)
            return t[i];
        else
            return _at<is...>(t[i]);
    }
};


}  // namespace traits


namespace concepts {

template<typename T>
concept tensor
= std::is_default_constructible_v<std::remove_cvref_t<T>>  // TODO: can we relax this?
and is_complete_v<traits::shape_of<std::remove_cvref_t<T>>>
and is_complete_v<xp::traits::scalar_type<std::remove_cvref_t<T>>>
and is_complete_v<traits::access<std::remove_cvref_t<T>>>
and requires(const T& t) {
    { traits::access<std::remove_cvref_t<T>>::at( *(md_index_iterator{traits::shape_of_t<std::remove_cvref_t<T>>{}}), t ) };
};

}  // namespace concepts

//! Compute the matrix product of two tensors
template<concepts::tensor T1, concepts::tensor T2>
inline constexpr auto mat_mul(const T1& t1, const T2& t2) noexcept {
    using shape1 = traits::shape_of_t<T1>;
    using shape2 = traits::shape_of_t<T2>;
    static_assert(shape1::dimensions > 1, "First argument must be a tensor with 2 or more dimensions.");
    static_assert(shape1{}.last() == shape2{}.first(), "Tensor dimensions do not match.");

    using scalar = std::common_type_t<
        xp::traits::scalar_type_t<T1>,
        xp::traits::scalar_type_t<T2>
    >;
    constexpr md_shape new_shape{
        typename shape1::as_values_t{}.template crop<1>()
        + typename shape2::as_values_t{}.template drop<1>()
    };

    // todo: deduce return tensor type somehow?
    linalg::tensor<scalar, decltype(new_shape)> result{scalar{0}};
    visit_indices_in(new_shape, [&] <std::size_t... i> (const md_index<i...>& idx) constexpr {
        visit_indices_in(shape<shape1{}.last()>, [&] <std::size_t j> (const md_index<j>&) constexpr {
            const auto t1_idx = md_index{values<i...>::template take<shape1::dimensions-1>() + values<j>{}};
            const auto t2_idx = md_index{values<j>{} + values<i...>::template drop<shape1::dimensions-1>()};
            result[idx] += traits::access<T1>::at(t1_idx, t1)*traits::access<T2>::at(t2_idx, t2);
        });
    });
    return result;
}

//! Return the determinant of the given tensor
template<concepts::tensor T>
    requires(traits::shape_of_t<T>{}.dimensions == 2)
inline constexpr auto determinant_of(const T& tensor) noexcept {
    static constexpr auto rows = traits::shape_of_t<T>{}.at(ic<0>);
    static constexpr auto cols = traits::shape_of_t<T>{}.at(ic<1>);
    static_assert(rows == cols, "Determinant can only be computed for square matrices.");
    static_assert(rows == 2 || rows == 3, "Determinant is only implemented for 2d & 3d matrices.");

    const auto _get = [&] <std::size_t... i> (const md_index<i...>& idx) constexpr noexcept {
        return linalg::traits::access<T>::at(idx, tensor);
    };

    if constexpr (rows == 2)
        return _get(at<0, 0>())*_get(at<1, 1>()) - _get(at<1, 0>())*_get(at<0, 1>());
    else
        return _get(at<0, 0>())*_get(at<1, 1>())*_get(at<2, 2>())
            + _get(at<0, 1>())*_get(at<1, 2>())*_get(at<2, 0>())
            + _get(at<0, 2>())*_get(at<1, 0>())*_get(at<2, 1>())
            - _get(at<0, 2>())*_get(at<1, 1>())*_get(at<2, 0>())
            - _get(at<0, 1>())*_get(at<1, 0>())*_get(at<2, 2>())
            - _get(at<0, 0>())*_get(at<1, 2>())*_get(at<2, 1>());
}

//! \} group LinearAlgebra

}  // namespace xp::linalg


// TODO: make these free functions and inject to default somehow? To make overloadable...
namespace xp::operators::traits {

template<linalg::concepts::tensor T, typename S> requires(xp::traits::is_scalar_v<S>)
struct multiplication_of<T, S> {
    template<same_remove_cvref_t_as<T> _T, same_remove_cvref_t_as<S> _S>
    constexpr T operator()(_T&& tensor, _S&& scalar) const noexcept {
        T result;
        visit_indices_in(linalg::traits::shape_of_t<T>{}, [&] (const auto& idx) {
            xp::traits::scalar_type_t<T>& value_at_idx = xp::linalg::traits::access<T>::at(idx, result);
            value_at_idx = xp::linalg::traits::access<T>::at(idx, tensor)*scalar;
        });
        return result;
    }
};
template<typename S, linalg::concepts::tensor T> requires(xp::traits::is_scalar_v<S>)
struct multiplication_of<S, T> {
    template<same_remove_cvref_t_as<S> _S, same_remove_cvref_t_as<T> _T>
    constexpr T operator()(_S&& scalar, _T&& tensor) const noexcept {
        return multiplication_of<T, S>{}(std::forward<_T>(tensor), std::forward<_S>(scalar));
    }
};

template<linalg::concepts::tensor T, typename S> requires(xp::traits::is_scalar_v<S>)
struct division_of<T, S> {
    template<same_remove_cvref_t_as<T> _T, same_remove_cvref_t_as<S> _S>
    constexpr T operator()(_T&& tensor, _S&& scalar) const noexcept {
        T result;
        visit_indices_in(linalg::traits::shape_of_t<T>{}, [&] (const auto& idx) {
            xp::traits::scalar_type_t<T>& value_at_idx = xp::linalg::traits::access<T>::at(idx, result);
            value_at_idx = xp::linalg::traits::access<T>::at(idx, tensor)/scalar;
        });
        return result;
    }
};

template<linalg::concepts::tensor T1, linalg::concepts::tensor T2>
    requires(linalg::traits::shape_of_t<T1>{} == linalg::traits::shape_of_t<T2>{})
struct multiplication_of<T1, T2> {
    template<same_remove_cvref_t_as<T1> _T1, same_remove_cvref_t_as<T2> _T2>
    constexpr auto operator()(_T1&& A, _T2&& B) const noexcept {
        xp::traits::scalar_type_t<T1> result{0};
        visit_indices_in(linalg::traits::shape_of_t<T1>{}, [&] (const auto& idx) {
            result += xp::linalg::traits::access<T1>::at(idx, A)
                        *xp::linalg::traits::access<T2>::at(idx, B);
        });
        return result;
    }
};

template<linalg::concepts::tensor T1, linalg::concepts::tensor T2>
    requires(linalg::traits::shape_of_t<T1>{} == linalg::traits::shape_of_t<T2>{})
struct addition_of<T1, T2> {
    template<same_remove_cvref_t_as<T1> _T1, same_remove_cvref_t_as<T2> _T2>
    constexpr auto operator()(_T1&& A, _T2&& B) const noexcept {
        using scalar = std::common_type_t<xp::traits::scalar_type_t<T1>, xp::traits::scalar_type_t<T2>>;
        using shape = linalg::traits::shape_of_t<T1>;
        linalg::tensor<scalar, shape> result{};
        visit_indices_in(shape{}, [&] (const auto& idx) {
            result[idx] = xp::linalg::traits::access<T1>::at(idx, A) + xp::linalg::traits::access<T2>::at(idx, B);
        });
        return result;
    }
};

template<linalg::concepts::tensor T1, linalg::concepts::tensor T2>
    requires(linalg::traits::shape_of_t<T1>{} == linalg::traits::shape_of_t<T2>{})
struct subtraction_of<T1, T2> {
    template<same_remove_cvref_t_as<T1> _T1, same_remove_cvref_t_as<T2> _T2>
    constexpr auto operator()(_T1&& A, _T2&& B) const noexcept {
        using scalar = std::common_type_t<xp::traits::scalar_type_t<T1>, xp::traits::scalar_type_t<T2>>;
        using shape = linalg::traits::shape_of_t<T1>;
        linalg::tensor<scalar, shape> result{};
        visit_indices_in(shape{}, [&] (const auto& idx) {
            result[idx] = xp::linalg::traits::access<T1>::at(idx, A) - xp::linalg::traits::access<T2>::at(idx, B);
        });
        return result;
    }
};

}  // namespace xp::operators::traits


namespace xp::traits {

template<typename T, typename shape>  // TODO: constrain on scalar T
struct scalar_type<linalg::tensor<T, shape>> : std::type_identity<T> {};

}  // namespace xp::traits
