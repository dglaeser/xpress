// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Operators
 * \brief Mathematical operations on symbols/expressions.
 */
#pragma once

#include <type_traits>

#include "../utils.hpp"
#include "../traits.hpp"


namespace xp {

//! \addtogroup Operators
//! \{

namespace operators {

namespace traits {

template<typename op>
struct is_commutative : std::false_type {};

}  // namespace traits

template<typename op>
inline constexpr bool is_commutative_v = traits::is_commutative<op>::value;

//! Base class that may be reused by operator implementations
template<template<typename...> typename trait, typename default_operator>
struct operator_base {
    template<typename... T>
    constexpr decltype(auto) operator()(T&&... t) const noexcept {
        if constexpr (is_complete_v<trait<std::remove_cvref_t<T>...>>)
            return trait<std::remove_cvref_t<T>...>{}(std::forward<T>(t)...);
        else {
            static_assert(
                requires(T&&... t) { { default_operator{}(std::forward<T>(t)...) }; },
                "Default operator cannot be invoked with the given types. "
                "Please specialize the respective trait."
            );
            return default_operator{}(std::forward<T>(t)...);
        }
    }
};

}  // namespace operators

//! Represents an expression resulting from an operator applied to the given terms
template<typename op, expression... Ts>
struct operation : bindable<traits::common_dtype_t<traits::dtype_of_t<Ts>...>>, negatable {
    using bindable<traits::common_dtype_t<traits::dtype_of_t<Ts>...>>::operator=;

    constexpr operation() = default;
    constexpr operation(const op&, const Ts&...) noexcept {}
};

template<typename op, typename... Ts>
operation(op&&, Ts&&...) -> operation<std::remove_cvref_t<op>, std::remove_cvref_t<Ts>...>;


namespace traits {

template<typename op, typename T1, typename T2>
    requires(operators::is_commutative_v<op>)
struct is_equal_node<operation<op, T1, T2>, operation<op, T2, T1>> : std::true_type {};

template<typename op, typename T, typename... Ts>
struct nodes_of<operation<op, T, Ts...>> {
    using type = merged_t<type_list<operation<op, T, Ts...>>, merged_nodes_of_t<T, Ts...>>;
};

template<typename op, typename... Ts>
struct value_of<operation<op, Ts...>> {
    template<typename... V>
    static constexpr decltype(auto) from(const bindings<V...>& binders) noexcept {
        using self = operation<op, Ts...>;
        if constexpr (bindings<V...>::template has_bindings_for<self>)
            return binders[self{}];
        else
            return op{}(xp::value_of(Ts{}, binders)...);
    }
};

}  // namespace traits

//! \} group Operators

}  // namespace xp
