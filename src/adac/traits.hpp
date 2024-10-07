// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Traits
 * \brief Type traits for symbols, expressions and values.
 */
#pragma once

#include <concepts>
#include <type_traits>

#include "utils.hpp"


namespace adac::traits {

//! \addtogroup Traits
//! \{

//! Register a type to be a scalar
template<typename T>
struct is_scalar : std::bool_constant<std::is_floating_point_v<T> || std::is_integral_v<T>> {};
template<typename T>
inline constexpr bool is_scalar_v = is_scalar<T>::value;

//! Trait to extract the value type (i.e. the scalar type) of e.g. a container
template<typename T>
struct value_type;
template<typename T> requires(is_scalar<T>::value)
struct value_type<T> : std::type_identity<T> {};
template<typename T>
using value_type_t = typename value_type<T>::type;

//! Trait to register a type as a value, i.e. a value that can be bound to a symbol/expression
template<typename T>
struct is_value : is_complete<value_type<T>> {};
template<typename T>
inline constexpr bool is_value_v = is_value<T>::value;

//! Trait to specify if an instance of type Arg can be bound for a symbol with data type T
template<typename T, typename Arg>
struct is_bindable : std::is_same<T, std::remove_cvref_t<Arg>> {};
template<std::floating_point T, std::integral Arg>
struct is_bindable<T, Arg> : std::is_convertible<Arg, T> {};
template<typename T, typename Arg>
inline constexpr bool is_bindable_v = is_bindable<T, Arg>::value;

//! Trait to extract the data type used for representing instances of type T
template<typename T>
struct dtype_of;
template<typename T>
using dtype_of_t = typename dtype_of<T>::type;

//! Trait to get the common data type for the given data types
template<typename... Ts>
struct common_dtype;
template<typename T>
struct common_dtype<T> : std::type_identity<T> {};
template<typename T>
struct common_dtype<T, T> : std::type_identity<T> {};
template<typename T1, typename T2, typename... Ts> requires(sizeof...(Ts) > 0)
struct common_dtype<T1, T2, Ts...> : common_dtype<typename common_dtype<T1, T2>::type, Ts...> {};
template<typename... Ts>
using common_dtype_t = typename common_dtype<Ts...>::type;

//! \} group Traits

}  // namespace adac::traits

namespace adac::concepts {

template<typename Arg, typename T>
concept bindable_to = is_complete_v<traits::is_bindable<T, Arg>> and traits::is_bindable<T, Arg>::value;

}  // namespace adac::concepts
