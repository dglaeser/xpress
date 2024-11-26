// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
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
#include <array>
#include <tuple>

#include "utils.hpp"
#include "traits.hpp"
#include "concepts.hpp"


namespace xp::linalg {

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

    template<typename S> requires(shape::dimensions == 1)
    constexpr decltype(auto) operator[](this S&& self, const std::size_t idx) noexcept {
        return self._values[idx];
    }

    template<typename S, std::integral... is> requires(shape::dimensions > 1 and sizeof...(is) == shape::dimensions)
    constexpr decltype(auto) operator[](this S&& self, const is&... indices) noexcept {
        const auto index_tuple = std::forward_as_tuple(indices...);
        const auto flat_index = [&] <std::size_t i0, std::size_t s0, std::size_t... s> (
            this auto&& self,
            const index_constant<i0>&,
            const md_shape<s0, s...>&,
            std::size_t accumulated
        ) {
            if constexpr (sizeof...(s) > 0)
                return self(ic<i0+1>, md_shape<s...>{}, accumulated + std::get<i0>(index_tuple)*md_shape<s...>::count);
            else
                return accumulated + std::get<i0>(index_tuple);
        };
        return self._values[flat_index(ic<0>, shape{}, 0)];
    }

    template<typename V> requires(is_scalar_v<V>)
    constexpr auto operator*(const V& value) const noexcept {
        auto scaled = _values;
        std::ranges::for_each(scaled, [&] (auto& v) { v *= value; });
        return linalg::tensor{shape{}, std::move(scaled)};
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

    template<typename _T> requires(!std::is_const_v<T> and is_complete_v<access<_T>>)
    constexpr void export_to(_T& out) const noexcept {
        visit_indices_in(shape{}, [&] (const auto& idx) constexpr {
            access<_T>::at(idx, out) = (*this)[idx];
        });
    }

 private:
    std::array<T, shape::count> _values;
};

template<std::size_t... s, typename T, std::size_t size>
    requires(md_shape<s...>::count == size)
tensor(const md_shape<s...>&, std::array<T, size>&&) -> tensor<T, md_shape<s...>>;

template<std::size_t... s, typename... Ts>
    requires(std::conjunction_v<is_scalar<std::remove_cvref_t<Ts>>...>)
tensor(const md_shape<s...>&, Ts&&...) -> tensor<std::common_type_t<Ts...>, md_shape<s...>>;

template<std::size_t... s, typename... Ts>
    requires(sizeof...(Ts) > 0 and !std::conjunction_v<is_scalar<std::remove_cvref_t<Ts>>...>)
tensor(const md_shape<s...>&, Ts&&...) -> tensor<std::remove_cvref_t<first_t<type_list<Ts...>>>, md_shape<s...>>;


//! Compute the matrix product of two tensors
template<tensorial T1, tensorial T2>
inline constexpr auto mat_mul(const T1& t1, const T2& t2) noexcept {
    using shape1 = shape_of_t<T1>;
    using shape2 = shape_of_t<T2>;
    static_assert(shape1::dimensions > 1, "First argument must be a tensor with 2 or more dimensions.");
    static_assert(shape1{}.last() == shape2{}.first(), "Tensor dimensions do not match.");

    constexpr md_shape new_shape{
        typename shape1::as_values_t{}.template crop<1>()
        + typename shape2::as_values_t{}.template drop<1>()
    };

    // todo: deduce return tensor type somehow?
    using scalar = std::common_type_t<scalar_type_t<T1>, scalar_type_t<T2>>;
    linalg::tensor<scalar, decltype(new_shape)> result{scalar{0}};
    visit_indices_in(new_shape, [&] <std::size_t... i> (const md_index<i...>& idx) constexpr {
        visit_indices_in(shape<shape1{}.last()>, [&] <std::size_t j> (const md_index<j>&) constexpr {
            const auto t1_idx = md_index{values<i...>::template take<shape1::dimensions-1>() + values<j>{}};
            const auto t2_idx = md_index{values<j>{} + values<i...>::template drop<shape1::dimensions-1>()};
            result[idx] += access<T1>::at(t1_idx, t1)*access<T2>::at(t2_idx, t2);
        });
    });
    return result;
}

//! Return the determinant of the given tensor
template<tensorial T>
    requires(shape_of_t<T>{}.dimensions == 2)
inline constexpr auto determinant_of(const T& tensor) noexcept {
    static constexpr auto rows = shape_of_t<T>{}.at(ic<0>);
    static constexpr auto cols = shape_of_t<T>{}.at(ic<1>);
    static_assert(rows == cols, "Determinant can only be computed for square matrices.");
    static_assert(rows == 2 || rows == 3, "Determinant is only implemented for 2x2 & 3x3 matrices.");

    const auto _get = [&] <std::size_t... i> (const md_index<i...>& idx) constexpr noexcept {
        return access<T>::at(idx, tensor);
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

namespace xp {

template<typename T, typename shape>  // TODO: constrain on scalar T
struct scalar_type<linalg::tensor<T, shape>> : std::type_identity<T> {};

#ifndef DOXYGEN
namespace detail {

    template<typename T>
    concept has_static_size = requires {
        { T::size } -> std::convertible_to<std::size_t>;
    };

    template<typename T>
    struct size_of;
    template<typename T, std::size_t s>
    struct size_of<std::array<T, s>> : std::integral_constant<std::size_t, s> {};
    template<detail::has_static_size T>
    struct size_of<T> : std::integral_constant<std::size_t, T::size> {};
    template<typename T> requires(is_complete_v<size_of<T>>)
    inline constexpr std::size_t size_of_v = size_of<T>::value;

    template<typename T, std::size_t... s>
    struct shape_of_indexable;
    template<typename T, std::size_t... s> requires(!is_indexable_v<T>)
    struct shape_of_indexable<T, s...> : std::type_identity<md_shape<s...>> {};
    template<typename T, std::size_t... s> requires(is_indexable_v<T>)
    struct shape_of_indexable<T, s...> : shape_of_indexable<value_type_t<T>, s..., size_of_v<T>> {};

}  // namespace detail
#endif  // DOXYGEN

template<typename T>
struct shape_of;
template<typename T> requires(is_indexable_v<T> and is_complete_v<detail::size_of<T>>)
struct shape_of<T> : detail::shape_of_indexable<T> {};
template<typename T, typename shape>
struct shape_of<linalg::tensor<T, shape>> : std::type_identity<shape> {};
template<typename T>
using shape_of_t = typename shape_of<T>::type;

template<typename T>
struct access;
template<typename T, typename shape>
struct access<linalg::tensor<T, shape>> {
    template<same_remove_cvref_t_as<linalg::tensor<T, shape>> _T, std::size_t... i>
    static constexpr decltype(auto) at(const md_index<i...>& idx, _T&& tensor) noexcept {
        return tensor[idx];
    }
};
template<typename T> requires(is_indexable_v<T> and is_complete_v<shape_of<T>>)
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

}  // namespace xp
