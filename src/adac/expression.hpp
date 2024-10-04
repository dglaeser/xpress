// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Expressions
 * \brief Interface for expressions.
 */
#pragma once

#include <utility>
#include <ostream>
#include <concepts>
#include <type_traits>

#include <adac/utils.hpp>
#include <adac/dtype.hpp>
#include <adac/bindings.hpp>
#include <adac/values.hpp>
#include <adac/expression_traits.hpp>

namespace adac {

//! \addtogroup Expressions
//! \{

//! Base class for negatable symbols/expressions
struct negatable {
    template<typename Self>
    constexpr auto operator-(this Self&& self) {
        return value<-1>{}*std::forward<Self>(self);
    }
};

//! base class for bindable symbols/expressions
template<concepts::dtype T = dtype::any>
struct bindable {
    using dtype = T;

    //! bind the given value to this symbol
    template<typename Self, typename V> requires(concepts::accepts<T, V>)
    constexpr auto operator=(this Self&& self, V&& value) noexcept {
        return value_binder(std::forward<Self>(self), std::forward<V>(value));
    }
};


namespace concepts {

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

template<typename E, typename... V>
    requires(concepts::streamable_with<E, V...>)
inline constexpr void write_to(std::ostream& out, const E&, const bindings<V...>& values) noexcept {
    traits::stream<E>::to(out, values);
}

//! Create a type_list containing the given terms
template<typename... V>
inline constexpr auto wrt(const V&...) {
    return type_list<V...>{};
}

//! All nodes in the given expression
template<typename T> requires(is_complete_v<traits::nodes_of<T>>)
using nodes_of_t = traits::nodes_of<T>::type;

//! All leaf nodes in the given expresssion
template<typename T>
using leaf_nodes_of_t = filtered_types_t<traits::is_leaf_node, nodes_of_t<T>>;

//! All non-leaf nodes in the given expression
template<typename T>
using composite_nodes_of_t = filtered_types_t<traits::is_composite_node, nodes_of_t<T>>;


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

    template<typename T>
    struct common_dtype_of;
    template<typename T, typename... Ts>
    struct common_dtype_of<type_list<T, Ts...>> {
        using type = dtype::common_dtype_of_t<T, Ts...>;
    };

}  // namespace detail
#endif  // DOXYGEN

//! All unique nodes in the given expression
template<typename T> requires(is_complete_v<traits::nodes_of<T>>)
using unique_nodes_of_t = detail::unique_nodes_of<nodes_of_t<T>>::type;

//! All leaf nodes in the given expresssion
template<typename T>
using unique_leaf_nodes_of_t = filtered_types_t<traits::is_leaf_node, unique_nodes_of_t<T>>;

//! All unique non-leaf nodes in the given expression
template<typename T>
using unique_composite_nodes_of_t = filtered_types_t<traits::is_composite_node, unique_nodes_of_t<T>>;

//! Deduce the dtype of the given expression
template<concepts::expression T>
using dtype_of_t = typename detail::common_dtype_of<unique_nodes_of_t<T>>::type;

//! \} group Expressions

}  // namespace adac
