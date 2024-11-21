// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Concepts
 * \brief Concepts for symbols, expressions and values.
 */
#pragma once

#include <type_traits>
#include <concepts>
#include <ostream>

#include "utils.hpp"
#include "traits.hpp"
#include "bindings.hpp"


namespace xp {

template<typename T>
struct is_expression : std::bool_constant<
    is_complete_v<traits::value_of<T>> && is_complete_v<traits::derivative_of<T>> && is_complete_v<traits::nodes_of<T>>
> {};
template<typename T>
inline constexpr bool is_expression_v = is_expression<T>::value;


namespace concepts {
//! \addtogroup Concepts
//! \{

template<typename Arg, typename T>
concept bindable_to = is_complete_v<traits::is_bindable<T, Arg>> and traits::is_bindable<T, Arg>::value;

template<typename T, typename... V>
concept evaluatable_with = is_complete_v<traits::value_of<T>> and requires(const bindings<V...>& values) {
    { traits::value_of<T>::from(values) };
};

template<typename T, typename V>
concept differentiable_wrt = is_complete_v<traits::derivative_of<T>> and requires(const type_list<V>& var) {
    { traits::derivative_of<T>::wrt(var) };
};

template<typename T, typename... V>
concept streamable_with = is_complete_v<traits::stream<T>> and requires(const T&, std::ostream& out, const bindings<V...>& values) {
    { traits::stream<T>::to(out, values) } -> std::same_as<void>;
};

template<typename T>
concept symbol = traits::is_symbol_v<T>;

template<typename T>
concept variable = traits::is_variable_v<T>;

template<typename T>
concept expression = is_expression_v<T>;


//! \} group Concepts

}  // namespace concepts
}  // namespace xp
