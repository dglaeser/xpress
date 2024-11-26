// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Expressions
 * \brief Traits for symbols, expressions and values.
 */
#pragma once

#include <type_traits>
#include <concepts>
#include <ostream>

#include "type_traits.hpp"


namespace xp::traits {

//! \addtogroup Expressions
//! \{

//! Trait to register a type as a value, i.e. a value that can be bound to a symbol/expression
template<typename T>
struct is_value : is_complete<scalar_type<T>> {};
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
template<typename T> requires(requires { typename T::dtype; })
struct dtype_of<T> : std::type_identity<typename T::dtype> {};
template<typename T>
using dtype_of_t = typename dtype_of<T>::type;

//! Trait to get the common data type for the given data types (binary trait has to be specialized for dtypes)
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

//! Trait (metafunction) to get the value of an expression
template<typename T> struct value_of;

//! Trait (metafunction) to get the derivative of an expression wrt to a variable
template<typename T> struct derivative_of;

//! Trait (metafunction) to write an expression to an output stream
template<typename T> struct stream;

//! Trait to register a type as a symbol, i.e. a leaf node in an expression tree to which values can be bound
template<typename T>
struct is_symbol : std::false_type {};
template<typename T>
inline constexpr bool is_symbol_v = is_symbol<T>::value;

//! Trait to register a type as a variable, i.e. a symbol that represents an independent variable of an expression
template<typename T>
struct is_variable : std::false_type {};
template<typename T>
inline constexpr bool is_variable_v = is_variable<T>::value;

//! Trait to get a list of all nodes in an expression tree
template<typename T>
struct nodes_of;
template<typename T>
using nodes_of_t = typename nodes_of<T>::type;

//! Trait to compare two expressions for equality (can be specialized e.g. for commutative operators)
template<typename A, typename B>
struct is_equal_node : std::is_same<A, B> {};
template<typename A, typename B>
inline constexpr bool is_equal_node_v = is_equal_node<A, B>::value;

//! Trait to determine if a node is a leaf node (defaults to checking if there is only one node in its expression tree)
template<typename T>
struct is_leaf_node : std::bool_constant<nodes_of_t<T>::size == 1> {};
template<typename T>
inline constexpr bool is_leaf_node_v = is_leaf_node<T>::value;

//! Trait to determine if a node is a composite node (i.e. not a leaf node)
template<typename T>
struct is_composite_node : std::bool_constant<!is_leaf_node_v<T>> {};
template<typename T>
inline constexpr bool is_composite_node_v = is_composite_node<T>::value;

//! All leaf nodes in the given expresssion
template<typename T>
struct leaf_nodes_of : std::type_identity<filtered_t<traits::is_leaf_node, nodes_of_t<T>>> {};
template<typename T>
using leaf_nodes_of_t = typename leaf_nodes_of<T>::type;

//! All non-leaf nodes in the given expression
template<typename T>
struct composite_nodes_of : std::type_identity<filtered_t<traits::is_composite_node, nodes_of_t<T>>> {};
template<typename T>
using composite_nodes_of_t = typename composite_nodes_of<T>::type;


#ifndef DOXYGEN
namespace detail {

    template<typename merged, typename... Es>
    struct merged_nodes_of;
    template<typename... R>
    struct merged_nodes_of<type_list<R...>> : std::type_identity<type_list<R...>> {};
    template<typename... R, typename E0, typename... Es>
    struct merged_nodes_of<type_list<R...>, E0, Es...> {
        using type = typename merged_nodes_of<
            merged_t<type_list<R...>, typename nodes_of<E0>::type>,
            Es...
        >::type;
    };

    template<typename T>
    struct unique_nodes_of;

    template<typename T, typename... Ts>
    struct unique_nodes_of<type_list<T, Ts...>> {
        using type = std::conditional_t<
            std::disjunction_v<traits::is_equal_node<T, Ts>...>,
            typename unique_nodes_of<type_list<Ts...>>::type,
            merged_t<type_list<T>, typename unique_nodes_of<type_list<Ts...>>::type>
        >;
    };

    template<>
    struct unique_nodes_of<type_list<>> {
        using type = type_list<>;
    };

    template<typename T>
    struct common_dtype_of;
    template<typename T, typename... Ts>
    struct common_dtype_of<type_list<T, Ts...>> {
        using type = traits::common_dtype_t<traits::dtype_of_t<T>, traits::dtype_of_t<Ts>...>;
    };

}  // namespace detail
#endif  // DOXYGEN

//! Trait to get the merged list of nodes of all given expressions
template<typename... T> requires(std::conjunction_v<is_complete<nodes_of<T>>...>)
struct merged_nodes_of : detail::merged_nodes_of<type_list<>, T...> {};
template<typename... T>
using merged_nodes_of_t = typename merged_nodes_of<T...>::type;

//! All unique nodes in the given expression
template<typename T>
struct unique_nodes_of : std::type_identity<typename detail::unique_nodes_of<nodes_of_t<T>>::type> {};
template<typename T>
using unique_nodes_of_t = typename unique_nodes_of<T>::type;

//! All leaf nodes in the given expresssion
template<typename T>
struct unique_leaf_nodes_of : std::type_identity<filtered_t<traits::is_leaf_node, unique_nodes_of_t<T>>> {};
template<typename T>
using unique_leaf_nodes_of_t = typename unique_leaf_nodes_of<T>::type;

//! All unique non-leaf nodes in the given expression
template<typename T>
struct unique_composite_nodes_of : std::type_identity<filtered_t<traits::is_composite_node, unique_nodes_of_t<T>>> {};
template<typename T>
using unique_composite_nodes_of_t = typename unique_composite_nodes_of<T>::type;

//! All symbols in the given expression
template<typename T>
struct symbols_of : std::type_identity<filtered_t<traits::is_symbol, unique_leaf_nodes_of_t<T>>> {};
template<typename T>
using symbols_of_t = typename symbols_of<T>::type;

//! All variables in the given expression
template<typename T>
struct variables_of : std::type_identity<filtered_t<traits::is_variable, unique_leaf_nodes_of_t<T>>> {};
template<typename T>
using variables_of_t = typename variables_of<T>::type;

//! Trait to flag a type as representing a unit value
template<typename T>
struct is_unit_value : std::false_type {};
template<typename T>
inline constexpr bool is_unit_value_v = is_unit_value<T>::value;

//! Trait to flag a type as representing a value of zero
template<typename T>
struct is_zero_value : std::false_type {};
template<typename T>
inline constexpr bool is_zero_value_v = is_zero_value<T>::value;

//! Trait to check if a type implements the required expression traits
template<typename T>
struct is_expression : std::bool_constant<
    is_complete_v<traits::value_of<T>>
    and is_complete_v<traits::derivative_of<T>>
    and is_complete_v<traits::nodes_of<T>>
> {};
template<typename T>
inline constexpr bool is_expression_v = is_expression<T>::value;

//! \} group Expressions

}  // namespace xp::traits
