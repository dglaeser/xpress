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

#include "utils.hpp"
#include "dtype.hpp"
#include "bindings.hpp"
#include "values.hpp"
#include "traits.hpp"
#include "concepts.hpp"
#include "derivatives.hpp"


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


#ifndef DOXYGEN
namespace detail {

    template<typename E, typename V>
    inline constexpr auto differentiate(const type_list<V>&) noexcept {
        if constexpr (std::is_same_v<E, V>)
            return val<1>;
        else
            return traits::derivative_of<E>::wrt(type_list<V>{});
    }

}  // namespace detail
#endif  // DOXYGEN

//! Exposes an interface for the evaluation of an expression
template<concepts::expression E>
struct expression_evaluator {
    constexpr expression_evaluator(const E&) noexcept {}

    //! Evaluate the expression at the given values
    template<concepts::binder... V>
    constexpr auto at(V&&... values) const noexcept {
        return at(bindings{std::forward<V>(values)...});
    }

    //! Evaluate the expression at the given value bindings
    template<typename... V>
        requires(concepts::evaluatable_with<E, V...>)
    constexpr auto at(const bindings<V...>& values) const noexcept {
        return traits::value_of<E>::from(values);
    }
};

//! Exposes an interface for the differentiation of an expression
template<concepts::expression E>
struct expression_differentiator {
    constexpr expression_differentiator(const E&) noexcept {}

    //! Return the expression of the derivative w.r.t. to the given variable
    template<typename V>
    constexpr auto wrt(const V&) const noexcept {
        return detail::differentiate<E>(type_list<V>{});
    }

    //! Evaluate the expressions of the derivatives w.r.t. the given variables
    template<typename... V>
    constexpr auto wrt_n(V&&... vars) const noexcept {
        return derivatives{derivative{wrt(vars), vars}...};
    }
};


//! Return an evaluator for the given expression
template<typename E>
inline constexpr auto value_of(const E& expr) noexcept {
    return expression_evaluator{expr};
}

//! Evaluate the given expression from the given value bindings
template<typename E, typename... V>
    requires(concepts::evaluatable_with<E, V...>)
inline constexpr auto value_of(const E& expr, const bindings<V...>& values) noexcept {
    return value_of(expr).at(values);
}

//! Return a differentiator for the given expression
template<typename E>
inline constexpr auto derivatives_of(const E& expr) noexcept {
    return expression_differentiator{expr};
}

//! Return the expression of the derivative of the given expression w.r.t the given variable
template<typename E, typename V>
inline constexpr auto derivative_of(const E& expr, const type_list<V>&) noexcept {
    return derivatives_of(expr).wrt(V{});
}

//! Return the derivative of the given expression w.r.t the given variable, evaluated at the given values
template<typename E, typename V, typename... B>
inline constexpr auto derivative_of(const E& expr, const type_list<V>& var, const bindings<B...>& vals) noexcept {
    return value_of(derivative_of(expr, var), vals);
}

//! Return the derivatives of the given expression w.r.t the given variables
template<typename E, typename... V>
inline constexpr auto derivatives_of(const E& expr, const type_list<V...>&) noexcept {
    return derivatives_of(expr).wrt_n(V{}...);
}

//! Return the derivatives of the given expression w.r.t the given variables, evaluated at the given values
template<typename E, typename... V, typename... B>
inline constexpr auto derivatives_of(const E& expr, const type_list<V...>& vars, const bindings<B...>& vals) noexcept {
    return derivatives_of(expr, vars).at(vals);
}

//! Return the gradient of the given expression, i.e. the derivatives w.r.t. all of its variables
template<typename E>
inline constexpr auto gradient_of(const E& expr) noexcept {
    return derivatives_of(expr, traits::variables_of_t<E>{});
}

//! Return the gradient of the given expression evaluated at the given values
template<typename E, typename... B>
inline constexpr auto gradient_of(const E& expr, const bindings<B...>& vals) noexcept {
    return gradient_of(expr).at(vals);
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
