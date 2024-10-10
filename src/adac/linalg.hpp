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

#include "utils.hpp"
#include "traits.hpp"


namespace adac::linalg {

//! \addtogroup LinearAlgebra
//! \{

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
    template<typename T, std::size_t... s> requires(!adac::traits::is_indexable_v<T>)
    struct shape_of_indexable<T, s...> : std::type_identity<md_shape<s...>> {};
    template<typename T, std::size_t... s> requires(adac::traits::is_indexable_v<T>)
    struct shape_of_indexable<T, s...> : shape_of_indexable<adac::traits::value_type_t<T>, s..., size_of_v<T>> {};

}  // namespace detail
#endif  // DOXYGEN

template<typename T>
struct shape_of;
template<typename T> requires(adac::traits::is_indexable_v<T>)
struct shape_of<T> : detail::shape_of_indexable<T> {};
template<typename T>
using shape_of_t = typename shape_of<T>::type;

}  // namespace traits

//! \} group LinearAlgebra

}  // namespace adac::linalg
