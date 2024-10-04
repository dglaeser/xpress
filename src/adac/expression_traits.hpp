// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Expressions
 * \brief Interface for expressions.
 */
#pragma once

#include <type_traits>

#include <adac/utils.hpp>

namespace adac::traits {

//! \addtogroup Expressions
//! \{

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

//! Trait to write an expression to an output stream
template<typename T> struct stream;

//! Trait to determine if a node is a leaf node (defaults to checking if the tree below this node has only one node)
template<typename T> struct is_leaf_node : std::bool_constant<type_list_size_v<typename nodes_of<T>::type> == 1> {};

//! Trait to determine if a node is a composite node (i.e. not a leaf node)
template<typename T> struct is_composite_node : std::bool_constant<!is_leaf_node<T>::value> {};

//! \} group Expressions

}  // namespace adac::traits
