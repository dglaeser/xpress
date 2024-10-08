// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup LinearAlgebra
 * \brief Algebraic operations on types that represent tensors or vectors.
 */
#pragma once

#include <cstddef>
#include <type_traits>
#include <functional>

#include "utils.hpp"


namespace adac::linalg {

//! \addtogroup LinearAlgebra
//! \{

//! Type to represent the shape of a tensor
template<std::size_t... s>
struct shape {
    static constexpr std::size_t size = sizeof...(s);
    static constexpr std::size_t count = value_list<s...>{}.reduce_with(std::multiplies{}, std::size_t{1});

    template<std::size_t... v> requires(size > 0)
    constexpr bool operator==(const shape<v...>&) const noexcept { return false; }
    constexpr bool operator==(const shape&) const noexcept { return true; }

    template<std::size_t _i> requires(_i < size)
    static constexpr std::size_t at(const index_constant<_i>& idx) noexcept {
        return value_list<s...>::at(idx);
    }
};

//! Type to represent a multi-dimensional index
template<std::size_t... i>
struct md_index {
    static constexpr std::size_t size = sizeof...(i);

    template<std::size_t... v> requires(size > 0)
    constexpr bool operator==(const md_index<v...>&) const noexcept { return false; }
    constexpr bool operator==(const md_index&) const noexcept { return true; }

    template<std::size_t _i> requires(_i < size)
    static constexpr auto at(const index_constant<_i>&) noexcept {
        return index_constant<value_list<i...>::at(index_constant<_i>{})>{};
    }

    template<std::size_t s0, std::size_t... s> requires(sizeof...(s) == size - 1)
    static constexpr auto as_flat_index_in(const shape<s0, s...>&) noexcept {
        return index<_sum_up_flat_index<0>(shape<s...>{}, 0)>;
    }

    static constexpr auto as_flat_index_in(const shape<>&) noexcept requires(size == 0) {
        return index<0>;
    }

 private:
    template<std::size_t _i, std::size_t s0, std::size_t... s>
    static constexpr std::size_t _sum_up_flat_index(const shape<s0, s...>& sub_shape, std::size_t current) noexcept {
        return _sum_up_flat_index<_i + 1>(
            shape<s...>{},
            current + at(index<_i>).value*value_list<s0, s...>{}.reduce_with(std::multiplies{}, std::size_t{1})
        );
    }

    template<std::size_t _i>
    static constexpr std::size_t _sum_up_flat_index(const shape<>&, std::size_t current) noexcept {
        return current + at(index<_i>).value;
    }
};


#ifndef DOXYGEN
namespace detail {

    template<typename shape>
    struct first_index_in;
    template<std::size_t... s>
    struct first_index_in<shape<s...>> : std::type_identity<md_index<(s*0)...>> {};

}  // namespace detail
#endif  // DOXYGEN

//! Iterator over multi-dimensional indices
template<typename shape, typename current = typename detail::first_index_in<shape>::type>
struct md_index_iterator;

template<std::size_t... s, std::size_t... i> requires(sizeof...(s) == sizeof...(i))
struct md_index_iterator<shape<s...>, md_index<i...>> {
    constexpr md_index_iterator(const shape<s...>&) noexcept {}
    constexpr md_index_iterator(const shape<s...>&, const md_index<i...>&) noexcept {}

    constexpr bool is_incrementable() const noexcept {
        return md_index<i...>::as_flat_index_in(shape<s...>{}).value < shape<s...>::count - 1;
    }

    constexpr auto incremented() const noexcept {
        static_assert(
            md_index<i...>::as_flat_index_in(shape<s...>{}).value < shape<s...>::count - 1,
            "End of the range has already been reached"
        );
        return linalg::md_index_iterator{shape<s...>{}, _incremented<sizeof...(s) - 1, false>(md_index<>{})};
    }

    constexpr md_index<i...> operator*() const noexcept {
        return {};
    }

 private:
    template<std::size_t _i, bool was_incremented, std::size_t... c>
    constexpr auto _incremented(md_index<c...> intermediate_result) const noexcept {
        constexpr std::size_t at_i = md_index<i...>::at(index<_i>).value;
        constexpr bool do_increment = !was_incremented && at_i < shape<s...>::at(index<_i>) - 1;
        constexpr bool do_zero = !was_incremented && at_i >= shape<s...>::at(index<_i>) - 1;
        constexpr auto update = [&] () constexpr {
            if constexpr (do_increment)
                return md_index<at_i+1, c...>{};
            else if constexpr (do_zero)
                return md_index<0, c...>{};
            else
                return md_index<at_i, c...>{};
        } ();

        if constexpr (_i > 0)
            return _incremented<_i - 1, was_incremented || do_increment>(update);
        else
            return update;
    }
};

template<typename shape>
md_index_iterator(const shape&) -> md_index_iterator<shape, typename detail::first_index_in<shape>::type>;

template<typename shape, typename index>
md_index_iterator(const shape&, const index&) -> md_index_iterator<shape, index>;


namespace traits {

template<typename T>
struct is_indexable : std::bool_constant< requires(const T& t) { {t[std::size_t{}] }; } > {};
template<typename T>
inline constexpr bool is_indexable_v = is_indexable<T>::value;


#ifndef DOXYGEN
namespace detail {

    template<std::size_t s>
    void _invoke_with_constexpr_size() {}

    template<typename T>
    concept has_constexpr_size = requires(const T& t) {
        { t.size() } -> std::convertible_to<std::size_t>;
        { _invoke_with_constexpr_size<T{}.size()>() };
    };

}  // namespace detail
#endif  // DOXYGEN

template<typename T>
struct size_of;
template<detail::has_constexpr_size T>
struct size_of<T> : std::integral_constant<std::size_t, T{}.size()> {};
template<typename T> requires(is_complete_v<size_of<T>>)
inline constexpr std::size_t size_of_v = size_of<T>::value;

template<typename T>
struct value_type_of;
template<typename T> requires(is_indexable_v<T>)
struct value_type_of<T> : std::type_identity<std::remove_cvref_t<decltype(T{}[std::size_t{0}])>> {};
template<typename T> requires(is_complete_v<value_type_of<T>>)
using value_type_of_t = value_type_of<T>::type;


#ifndef DOXYGEN
namespace detail {

    template<typename T, std::size_t... s>
    struct shape_of_indexable;
    template<typename T, std::size_t... s> requires(!is_indexable_v<T>)
    struct shape_of_indexable<T, s...> : std::type_identity<shape<s...>> {};
    template<typename T, std::size_t... s> requires(is_indexable_v<T> and is_complete_v<value_type_of<T>>)
    struct shape_of_indexable<T, s...> : std::type_identity<
        typename shape_of_indexable<value_type_of_t<T>, s..., size_of_v<T>>::type
    > {};

}  // namespace detail
#endif  // DOXYGEN

template<typename T>
struct shape_of;
template<typename T> requires(is_indexable_v<T>)
struct shape_of<T> : detail::shape_of_indexable<T> {};
template<typename T>
using shape_of_t = typename shape_of<T>::type;

}  // namespace traits

//! \} group LinearAlgebra

}  // namespace adac::linalg
