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
#include <adac/traits.hpp>
#include <adac/concepts.hpp>


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
template<typename T = dtype::any>
struct bindable {
    using dtype = T;

    //! bind the given value to this symbol
    template<typename Self, concepts::bindable_to<dtype> V>
    constexpr auto operator=(this Self&& self, V&& value) noexcept {
        return value_binder(std::forward<Self>(self), std::forward<V>(value));
    }
};


//! Evaluate the given expression from the given value bindings
template<typename E, typename... V>
    requires(concepts::evaluatable_with<E, V...>)
inline constexpr auto evaluate(const E&, const bindings<V...>& values) noexcept {
    return traits::value_of<E>::from(values);
}

//! Specialization for expressions w/o any symbols
template<typename E>
    requires(concepts::evaluatable_with<E>)
inline constexpr auto evaluate(const E&) noexcept {
    return traits::value_of<E>::from(bindings<>{});
}

//! Return the derivative expression of the given expression w.r.t. the given variable
template<typename E, typename V>
    requires(concepts::differentiable_wrt<E, V>)
inline constexpr auto differentiate(const E&, const type_list<V>& var) noexcept {
    return traits::derivative_of<E>::wrt(var);
}

//! Write the given expression to the given stream with the given value bindings
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

//! \} group Expressions

}  // namespace adac
