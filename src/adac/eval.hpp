// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Evaluation
 * \brief Interface for getting values, derivatives, etc.
 */
#pragma once

#include <adac/utils.hpp>
#include <adac/bindings.hpp>

namespace adac {

//! \addtogroup Evaluation
//! \{

namespace traits {

//! Trait to get the value of an expression
template<typename T> struct value_of;

//! Trait to get the derivative of an expression wrt to a variable
template<typename T> struct derivative_of;

}  // namespace traits


namespace concepts {

template<typename T, typename... V>
concept evaluatable_with = is_complete_v<traits::value_of<T>> and requires(const bindings<V...>& values) {
    { traits::value_of<T>::from(values) };
};

template<typename T, typename... V>
concept differentiable_wrt = is_complete_v<traits::derivative_of<T>> and requires(const type_list<V...>& vars) {
    { traits::derivative_of<T>::wrt(vars) };
};

template<typename T>
concept expression = is_complete_v<traits::value_of<T>> and is_complete_v<traits::derivative_of<T>>;

}  // namespace concepts


template<typename E, typename... V>
    requires(concepts::evaluatable_with<E, V...>)
inline constexpr auto evaluate(const E&, const bindings<V...>& values) noexcept {
    return traits::value_of<E>::from(values);
}

template<typename E>
    requires(concepts::evaluatable_with<E>)
inline constexpr auto evaluate(const E&) noexcept {
    return traits::value_of<E>::from(bindings<>{});
}

template<typename E, typename V>
    requires(concepts::differentiable_wrt<E, V>)
inline constexpr auto differentiate(const E&, const type_list<V>& var) noexcept {
    return traits::derivative_of<E>::wrt(var);
}

//! Create a type_list containing the given terms
template<typename... V>
inline constexpr auto wrt(const V&...) {
    return type_list<V...>{};
}

//! \} group Evaluation

}  // namespace adac
