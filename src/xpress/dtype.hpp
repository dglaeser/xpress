// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Utils
 * \brief Data structures to represent classes of value types.
 *        Can be used to constrain symbols to only allow binding them to certain types.
 */
#pragma once

#include <concepts>
#include <type_traits>

#include "utils.hpp"
#include "traits.hpp"
#include "type_traits.hpp"


namespace xp {

namespace dtype {

struct any {};
struct real {};
struct integral {};

}  // namespace dtype


namespace traits {

template<typename Arg>
struct is_bindable<dtype::any, Arg> : std::true_type {};
template<typename Arg>
struct is_bindable<dtype::integral, Arg> : std::is_integral<std::remove_cvref_t<Arg>> {};
template<typename Arg>
struct is_bindable<dtype::real, Arg> : std::bool_constant<
    std::is_floating_point_v<std::remove_cvref_t<Arg>> ||
    std::is_integral_v<std::remove_cvref_t<Arg>>
> {};
template<tensorial Arg>
struct is_bindable<dtype::real, Arg> : is_bindable<dtype::real, scalar_type_t<Arg>> {};
template<tensorial Arg>
struct is_bindable<dtype::integral, Arg> : is_bindable<dtype::integral, scalar_type_t<Arg>> {};

#ifndef DOXYGEN
namespace detail {

    template<typename T> requires(is_value_v<T>)
    using dtype_for = std::conditional_t<std::floating_point<scalar_type_t<T>>, dtype::real, dtype::integral>;

}  // namespace detail
#endif  // DOXYGEN

template<> struct common_dtype<dtype::real, dtype::integral> : std::type_identity<dtype::real> {};
template<> struct common_dtype<dtype::integral, dtype::real> : std::type_identity<dtype::real> {};

template<typename T> requires(is_value_v<T>) struct common_dtype<dtype::integral, T> : std::type_identity<detail::dtype_for<T>> {};
template<typename T> requires(is_value_v<T>) struct common_dtype<T, dtype::integral> : std::type_identity<detail::dtype_for<T>> {};

template<typename T> requires(is_value_v<T>) struct common_dtype<dtype::real, T> : std::type_identity<dtype::real> {};
template<typename T> requires(is_value_v<T>) struct common_dtype<T, dtype::real> : std::type_identity<dtype::real> {};

template<typename T> requires(not std::is_same_v<T, dtype::any>) struct common_dtype<T, dtype::any> : std::type_identity<dtype::any> {};
template<typename T> requires(not std::is_same_v<T, dtype::any>) struct common_dtype<dtype::any, T> : std::type_identity<dtype::any> {};

}  // namespace traits
}  // namespace xp
