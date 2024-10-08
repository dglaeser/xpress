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

#include "utils.hpp"


namespace adac::linalg {

//! \addtogroup LinearAlgebra
//! \{

//! Type to represent the shape of a tensor
template<std::size_t... s>
struct shape {

    template<std::size_t... v>
    constexpr bool operator==(const shape<v...>&) const noexcept { return false; }
    constexpr bool operator==(const shape&) const noexcept { return true; }
};


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
