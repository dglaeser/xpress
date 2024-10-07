// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Concepts
 * \brief Concepts for symbols, expressions and values.
 */
#pragma once

#include <concepts>
#include <ostream>

#include "utils.hpp"
#include "traits.hpp"
#include "bindings.hpp"


namespace adac::concepts {

//! \addtogroup Concepts
//! \{

template<typename Arg, typename T>
concept bindable_to = is_complete_v<traits::is_bindable<T, Arg>> and traits::is_bindable<T, Arg>::value;

template<typename T, typename... V>
concept evaluatable_with = is_complete_v<traits::value_of<T>> and requires(const bindings<V...>& values) {
    { traits::value_of<T>::from(values) };
};

template<typename T, typename... V>
concept differentiable_wrt = is_complete_v<traits::derivative_of<T>> and requires(const type_list<V...>& vars) {
    { traits::derivative_of<T>::wrt(vars) };
};

template<typename T, typename... V>
concept streamable_with = is_complete_v<traits::stream<T>> and requires(const T&, std::ostream& out, const bindings<V...>& values) {
    { traits::stream<T>::to(out, values) } -> std::same_as<void>;
};

template<typename T>
concept expression = is_complete_v<traits::value_of<T>> and is_complete_v<traits::derivative_of<T>> and is_complete_v<traits::nodes_of<T>>;


//! \} group Concepts

}  // namespace adac::concepts