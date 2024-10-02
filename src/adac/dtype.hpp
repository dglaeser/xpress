// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Utils
 * \brief Data structures to represent symbols from which expressions can be constructed.
 */
#pragma once

#include <concepts>
#include <type_traits>

#include <adac/utils.hpp>


namespace adac {
namespace dtype {

struct any {};
struct real {};
struct integral {};

template<typename T, typename Arg>
struct accepts : public std::is_same<T, std::remove_cvref_t<Arg>> {};
template<typename Arg>
struct accepts<any, Arg> : public std::true_type {};
template<typename Arg>
struct accepts<real, Arg> : public std::bool_constant<std::is_floating_point_v<std::remove_cvref_t<Arg>> or std::is_integral_v<std::remove_cvref_t<Arg>>> {};
template<typename Arg>
struct accepts<integral, Arg> : public std::bool_constant<std::is_integral_v<std::remove_cvref_t<Arg>>> {};

}  // namespace dtype


namespace concepts {

template<typename T, typename Arg>
concept accepts = is_complete_v<dtype::accepts<T, Arg>> and dtype::accepts<T, Arg>::value;

}  // namespace concepts
}  // namespace adac
