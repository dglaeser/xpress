// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Evaluation
 * \brief Interface for getting values, derivatives, etc.
 */
#pragma once

#include <type_traits>

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

//! Trait for expressions/symbol to specialize if arithmetic operators are defined in-class
template<typename A, typename B> struct disable_generic_arithmetic_operators : std::false_type {};

//! Trait to get a list of all nodes in this expression (tree)
template<typename T> struct nodes_of;

//! Trait to compare two expressions for equality (can be specialized e.g. for commutative operators)
template<typename A, typename B> struct is_equal_node : std::is_same<A, B> {};

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

//! All nodes in the given expression
template<typename T> requires(is_complete_v<traits::nodes_of<T>>)
using nodes_of_t = traits::nodes_of<T>::type;


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct unique_nodes_of;

    template<typename T, typename... Ts>
    struct unique_nodes_of<type_list<T, Ts...>> {
        using type = std::conditional_t<
            std::disjunction_v<traits::is_equal_node<T, Ts>...>,
            typename unique_nodes_of<type_list<Ts...>>::type,
            merged_types_t<type_list<T>, typename unique_nodes_of<type_list<Ts...>>::type>
        >;
    };

    template<>
    struct unique_nodes_of<type_list<>> {
        using type = type_list<>;
    };

}  // namespace detail
#endif  // DOXYGEN

//! All unique nodes in the given expression
template<typename T> requires(is_complete_v<traits::nodes_of<T>>)
using unique_nodes_of_t = detail::unique_nodes_of<nodes_of_t<T>>::type;

//! \} group Evaluation

}  // namespace adac
