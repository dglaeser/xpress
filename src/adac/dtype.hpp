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

namespace traits {

template<typename T>
struct dtype_of;

}  // namespace traits


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


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct is_dtype : std::integral_constant<bool, is_any_of_v<T, any, real, integral>> {};

    template<typename T1, typename... T2>
    struct common_dtype;
    template<typename T1, typename T2, typename T3, typename... Ts>
    struct common_dtype<T1, T2, T3, Ts...> : common_dtype<typename common_dtype<T1, T2>::type, T3, Ts...> {};

    template<typename T> struct common_dtype<T> : std::type_identity<T> {};
    template<typename T> struct common_dtype<T, T> : std::type_identity<T> {};
    template<> struct common_dtype<real, integral> : std::type_identity<real> {};
    template<> struct common_dtype<integral, real> : std::type_identity<real> {};

    template<typename T> requires(not std::is_same_v<T, any>) struct common_dtype<T, any> : std::type_identity<any> {};
    template<typename T> requires(not std::is_same_v<T, any>) struct common_dtype<any, T> : std::type_identity<any> {};

}  // namespace detail
#endif  // DOXYGEN

template<typename T1, typename... Ts>
    requires(detail::is_dtype<T1>::value and std::conjunction_v<detail::is_dtype<Ts>...>)
struct common_dtype : detail::common_dtype<T1, Ts...> {};

template<typename T1, typename... Ts>
using common_dtype_t = typename common_dtype<T1, Ts...>::type;

template<typename T1, typename... Ts>
    requires(is_complete_v<traits::dtype_of<T1>> and std::conjunction_v<is_complete<traits::dtype_of<Ts>>...>)
using common_dtype_of_t = common_dtype_t<typename traits::dtype_of<T1>::type, typename traits::dtype_of<Ts>::type...>;

}  // namespace dtype


namespace concepts {

template<typename T>
concept dtype = dtype::detail::is_dtype<T>::value;

template<typename T, typename Arg>
concept accepts = dtype<T> and is_complete_v<dtype::accepts<T, Arg>> and dtype::accepts<T, Arg>::value;

}  // namespace concepts
}  // namespace adac
