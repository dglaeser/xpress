// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
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


namespace xp {

//! \addtogroup Expressions
//! \{

//! An expression plus values bound to its symbols
template<expression E, typename... V>
struct bound_expression {
    explicit constexpr bound_expression(const E&, bindings<V...>&& bindings)
    : _bindings{std::move(bindings)}
    {}

    //! Return the value of the expression evaluated at the stored bindings
    constexpr auto value() const noexcept requires(evaluatable_with<E, V...>) {
        return traits::value_of<E>::from(_bindings);
    }

    //! Insert this expression into the given output stream
    friend constexpr std::ostream& operator<<(std::ostream& s, const bound_expression& e) noexcept {
        traits::stream<E>::to(s, e._bindings);
        return s;
    }

 private:
    bindings<V...> _bindings;
};

//! Base class for negatable symbols/expressions
struct negatable {
    template<typename Self>
    constexpr auto operator-(this Self&& self) {
        return value<-1>{}*std::forward<Self>(self);
    }
};

//! Base class for bindable symbols/expressions
template<typename T = dtype::any>
struct bindable {
    using dtype = T;

    //! Bind the given value to this symbol/expression
    template<typename Self, bindable_to<dtype> V>
    constexpr auto operator=(this Self&& self, V&& value) noexcept {
        return value_binder(std::forward<Self>(self), std::forward<V>(value));
    }

    //! Construct a bound expression from this expression and the given binders
    template<typename Self, binder... V>
    constexpr auto with(this Self&& self, V&&... binders) noexcept {
        return bound_expression{self, at(std::forward<V>(binders)...)};
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
template<expression E>
struct evaluator {
    constexpr evaluator(const E&) noexcept {}

    //! Evaluate the expression at the given (bound) values
    template<binder... V>
    constexpr decltype(auto) operator()(V&&... values) const noexcept {
        return at(bindings{std::forward<V>(values)...});
    }

    //! Evaluate the expression at the given value bindings
    template<typename... V>
        requires(evaluatable_with<E, V...>)
    constexpr decltype(auto) operator()(const bindings<V...>& values) const noexcept {
        return traits::value_of<E>::from(values);
    }

    //! Evaluate the expression at the given (bound) values
    template<binder... V>
    constexpr decltype(auto) at(V&&... values) const noexcept {
        return at(bindings{std::forward<V>(values)...});
    }

    //! Evaluate the expression at the given value bindings
    template<typename... V>
        requires(evaluatable_with<E, V...>)
    constexpr decltype(auto) at(const bindings<V...>& values) const noexcept {
        return traits::value_of<E>::from(values);
    }
};

//! Exposes an interface for the differentiation of an expression
template<expression E>
struct differentiator {
    constexpr differentiator(const E&) noexcept {}

    //! Return the expression of the derivative w.r.t. to the given variable
    template<differentiable_wrt<E> V>
    constexpr auto wrt(const V&) const noexcept {
        return detail::differentiate<E>(type_list<V>{});
    }

    //! Evaluate the expressions of the derivatives w.r.t. the given variables
    template<differentiable_wrt<E>... V>
    constexpr auto wrt_n(V&&... vars) const noexcept {
        return derivatives{derivative{wrt(vars), vars}...};
    }
};

//! Evaluate the given expression from the given value bindings
template<expression E, typename... V>
    requires(evaluatable_with<E, V...>)
inline constexpr decltype(auto) value_of(const E& expr, const bindings<V...>& values) noexcept {
    return evaluator{expr}.at(values);
}

//! Return the expression of the derivative of the given expression w.r.t the given variable
template<expression E, typename V>
inline constexpr auto derivative_of(const E& expr, const type_list<V>&) noexcept {
    return differentiator{expr}.wrt(V{});
}

//! Return the derivative of the given expression w.r.t the given variable, evaluated at the given values
template<expression E, typename V, typename... B>
inline constexpr decltype(auto) derivative_of(const E& expr, const type_list<V>& var, const bindings<B...>& vals) noexcept {
    return value_of(derivative_of(expr, var), vals);
}

//! Return the derivatives of the given expression w.r.t the given variables
template<expression E, typename... V>
inline constexpr auto derivatives_of(const E& expr, const type_list<V...>&) noexcept {
    return differentiator{expr}.wrt_n(V{}...);
}

//! Return the derivatives of the given expression w.r.t the given variables, evaluated at the given values
template<expression E, typename... V, typename... B>
inline constexpr auto derivatives_of(const E& expr, const type_list<V...>& vars, const bindings<B...>& vals) noexcept {
    return derivatives_of(expr, vars).at(vals);
}

//! Return the gradient of the given expression, i.e. the derivatives w.r.t. all of its variables
template<expression E>
inline constexpr auto gradient_of(const E& expr) noexcept {
    return derivatives_of(expr, traits::variables_of_t<E>{});
}

//! Return the gradient of the given expression evaluated at the given values
template<expression E, typename... B>
inline constexpr auto gradient_of(const E& expr, const bindings<B...>& vals) noexcept {
    return gradient_of(expr).at(vals);
}

//! Write the given expression to the given stream with the given value bindings
template<expression E, typename... V>
    requires(streamable_with<E, V...>)
inline constexpr void write_to(std::ostream& out, const E&, const bindings<V...>& values) noexcept {
    traits::stream<E>::to(out, values);
}

//! Create a type_list containing the given terms
template<typename... V>
inline constexpr auto wrt(const V&...) {
    return type_list<V...>{};
}

//! \} group Expressions

}  // namespace xp


#include <format>
#include <sstream>

template<typename E, typename... V>
struct std::formatter<xp::bound_expression<E, V...>> {

    // todo: precision formatter?
    template<typename parse_ctx>
    constexpr parse_ctx::iterator parse(parse_ctx& ctx) {
        auto it = ctx.begin();
        if (it == ctx.end())
            return it;
        if (*it != '}')
            throw std::format_error("xp::bound_expression does not support format args.");
        return it;
    }

    template<typename fmt_ctx>
    fmt_ctx::iterator format(const xp::bound_expression<E, V...>& e, fmt_ctx& ctx) const {
        std::ostringstream out;
        out << e;
        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};
