// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Operators
 * \brief Mathematical operations on symbols/expressions.
 */
#pragma once

#include <cmath>
#include <type_traits>
#include <functional>

#include "utils.hpp"
#include "traits.hpp"
#include "values.hpp"
#include "expressions.hpp"


namespace xp {

//! \addtogroup Operators
//! \{

namespace operators {

namespace traits {

template<typename op> struct is_commutative : std::false_type {};
template<typename A, typename B> struct addition_of;
template<typename A, typename B> struct subtraction_of;
template<typename A, typename B> struct multiplication_of;
template<typename A, typename B> struct division_of;
template<typename A, typename B> struct power_of;
template<typename A> struct log_of;

}  // namespace traits

template<typename op>
inline constexpr bool is_commutative_v = traits::is_commutative<op>::value;


#ifndef DOXYGEN
namespace detail {

    struct default_pow_operator {
        template<typename A, typename B>
        constexpr auto operator()(A&& a, B&& b) const noexcept {
            return std::pow(std::forward<A>(a), std::forward<B>(b));
        }
    };

    struct default_log_operator {
        template<typename A>
        constexpr auto operator()(A&& a) const noexcept {
            return std::log(std::forward<A>(a));
        }
    };

    template<template<typename> typename trait, typename default_t>
    struct unary_operator {
        template<typename T>
        constexpr decltype(auto) operator()(T&& t) const noexcept {
            if constexpr (is_complete_v<trait<std::remove_cvref_t<T>>>)
                return trait<std::remove_cvref_t<T>>{}(std::forward<T>(t));
            else {
                static_assert(
                    requires(T&& t) { { default_t{}(std::forward<T>(t)) }; },
                    "Default unary operator cannot be invoked with the given type. "
                    "Please specialize the respective trait in xp::operators::traits."
                );
                return default_t{}(std::forward<T>(t));
            }
        }
    };

    template<template<typename...> typename trait, typename default_t>
    struct binary_operator {
        template<typename... T>
        constexpr decltype(auto) operator()(T&&... t) const noexcept {
            if constexpr (is_complete_v<trait<std::remove_cvref_t<T>...>>)
                return trait<std::remove_cvref_t<T>...>{}(std::forward<T>(t)...);
            else {
                static_assert(
                    requires(T&&... t) { { default_t{}(std::forward<T>(t)...) }; },
                    "Default binary operator cannot be invoked with the given types. "
                    "Please specialize the respective trait in xp::operators::traits."
                );
                return default_t{}(std::forward<T>(t)...);
            }
        }
    };

}  // namespace detail
#endif  // DOXYGEN

struct add : detail::binary_operator<traits::addition_of, std::plus<void>> {};
struct subtract : detail::binary_operator<traits::subtraction_of, std::minus<void>> {};
struct multiply : detail::binary_operator<traits::multiplication_of, std::multiplies<void>> {};
struct divide : detail::binary_operator<traits::division_of, std::divides<void>> {};
struct pow : detail::binary_operator<traits::power_of, detail::default_pow_operator> {};
struct log : detail::unary_operator<traits::log_of, detail::default_log_operator> {};

namespace traits {

template<> struct is_commutative<add> : std::true_type {};
template<> struct is_commutative<multiply> : std::true_type {};

}  // namespace traits
}  // namespace operators


//! Class to represent expressions resulting from an operator applied to the given terms
template<typename op, concepts::expression... Ts>
struct operation : bindable<traits::common_dtype_t<traits::dtype_of_t<Ts>...>>, negatable {
    using bindable<traits::common_dtype_t<traits::dtype_of_t<Ts>...>>::operator=;

    constexpr operation() = default;
    constexpr operation(const op&, const Ts&...) noexcept {}
};

template<typename op, typename... Ts>
operation(op&&, Ts&&...) -> operation<std::remove_cvref_t<op>, std::remove_cvref_t<Ts>...>;


//! Add two expressions
template<concepts::expression A, concepts::expression B>
    requires(!traits::disable_generic_arithmetic_operators<A, B>::value)
inline constexpr auto operator+(const A&, const B&) noexcept {
    if constexpr (traits::is_zero_value_v<A>)
        return B{};
    else if constexpr (traits::is_zero_value_v<B>)
        return A{};
    else
        return operation<operators::add, A, B>{};
}

//! Subtract two expressions
template<concepts::expression A, concepts::expression B>
    requires(!traits::disable_generic_arithmetic_operators<A, B>::value)
inline constexpr auto operator-(const A&, const B&) noexcept {
    if constexpr (traits::is_zero_value_v<A>)
        return -B{};
    else if constexpr (traits::is_zero_value_v<B>)
        return A{};
    else
        return operation<operators::subtract, A, B>{};
}

//! Multiply two expressions
template<concepts::expression A, concepts::expression B>
    requires(!traits::disable_generic_arithmetic_operators<A, B>::value)
inline constexpr auto operator*(const A&, const B&) noexcept {
    if constexpr (traits::is_zero_value_v<A> || traits::is_zero_value_v<B>)
        return val<0>;
    else if constexpr (traits::is_unit_value_v<A>)
        return B{};
    else if constexpr (traits::is_unit_value_v<B>)
        return A{};
    else
        return operation<operators::multiply, A, B>{};
}

//! Divide two expressions
template<concepts::expression A, concepts::expression B>
    requires(!traits::disable_generic_arithmetic_operators<A, B>::value)
inline constexpr auto operator/(const A&, const B&) noexcept {
    static_assert(!traits::is_zero_value_v<B>, "Attempted division by zero.");
    if constexpr (traits::is_zero_value_v<A>)
        return val<0>;
    else if constexpr (traits::is_unit_value_v<B>)
        return A{};
    else
        return operation<operators::divide, A, B>{};
}

//! Take the natural logarithm of the given value
template<concepts::expression A>
inline constexpr auto log(const A&) noexcept {
    static_assert(!traits::is_zero_value_v<A>, "Logarithm of zero is not defined.");
    if constexpr (traits::is_unit_value_v<A>)
        return val<1>;
    else
        return operation<operators::log, A>{};
}

//! Take the power of the given value to the given exponent
template<concepts::expression A, concepts::expression B>
inline constexpr auto pow(const A&, const B&) noexcept {
    if constexpr (traits::is_zero_value_v<A>)
        return val<0>;
    else if constexpr (traits::is_unit_value_v<A> || traits::is_unit_value_v<B>)
        return A{};
    else if constexpr (traits::is_zero_value_v<B>)
        return val<1>;
    else
        return operation<operators::pow, A, B>{};
}

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


// traits for addition
template<typename T1, typename T2>
struct derivative_of<operation<operators::add, T1, T2>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& var) noexcept {
        return xp::detail::differentiate<T1>(var) + xp::detail::differentiate<T2>(var);
    }
};

template<typename T1, typename T2>
struct stream<operation<operators::add, T1, T2>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) noexcept {
        write_to(out, T1{}, values);
        out << " + ";
        write_to(out, T2{}, values);
    }
};


// traits for subtraction
template<typename T1, typename T2>
struct derivative_of<operation<operators::subtract, T1, T2>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& var) noexcept {
        return xp::detail::differentiate<T1>(var) - xp::detail::differentiate<T2>(var);
    }
};

template<typename T1, typename T2>
struct stream<operation<operators::subtract, T1, T2>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) noexcept {
        write_to(out, T1{}, values);
        out << " - ";
        write_to(out, T2{}, values);
    }
};


// traits for multiplication
template<typename T1, typename T2>
struct derivative_of<operation<operators::multiply, T1, T2>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& var) noexcept {
        return xp::detail::differentiate<T1>(var)*T2{} + T1{}*xp::detail::differentiate<T2>(var);
    }
};

template<typename T1, typename T2>
struct stream<operation<operators::multiply, T1, T2>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) noexcept {
        static constexpr bool has_subterms_1 = nodes_of_t<T1>::size > 1;
        if constexpr (has_subterms_1) out << "(";
        write_to(out, T1{}, values);
        if constexpr (has_subterms_1) out << ")";

        out << "*";

        static constexpr bool has_subterms_2 = nodes_of_t<T2>::size > 1;
        if constexpr (has_subterms_2) out << "(";
        write_to(out, T2{}, values);
        if constexpr (has_subterms_2) out << ")";
    }
};


// traits for division
template<typename T1, typename T2>
struct derivative_of<operation<operators::divide, T1, T2>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& var) noexcept {
        return xp::detail::differentiate<T1>(var)/T2{} - T1{}*xp::detail::differentiate<T2>(var)/(T2{}*T2{});
    }
};

template<typename T1, typename T2>
struct stream<operation<operators::divide, T1, T2>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) noexcept {
        static constexpr bool has_subterms_1 = nodes_of_t<T1>::size > 1;
        if constexpr (has_subterms_1) out << "(";
        write_to(out, T1{}, values);
        if constexpr (has_subterms_1) out << ")";

        out << "/";

        static constexpr bool has_subterms_2 = nodes_of_t<T2>::size > 1;
        if constexpr (has_subterms_2) out << "(";
        write_to(out, T2{}, values);
        if constexpr (has_subterms_2) out << ")";
    }
};

// traits for log operator
template<typename T>
struct derivative_of<operation<operators::log, T>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& var) noexcept {
        return xp::detail::differentiate<T>(var)/T{};
    }
};

template<typename T>
struct stream<operation<operators::log, T>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) noexcept {
        out << "log(";
        write_to(out, T{}, values);
        out << ")";
    }
};

// traits for power operator
template<typename T1, typename T2>
struct derivative_of<operation<operators::pow, T1, T2>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& var) noexcept {
        return T2{}*pow(T1{}, T2{} - val<1>)*xp::detail::differentiate<T1>(var)
            + pow(T1{}, T2{})*log(T2{})*xp::detail::differentiate<T2>(var);
    }
};

template<typename T1, typename T2>
struct stream<operation<operators::pow, T1, T2>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) noexcept {
        write_to(out, T1{}, values);
        out << "^";
        static constexpr bool exponent_has_subterms = nodes_of_t<T2>::size > 1;
        if constexpr (exponent_has_subterms) out << "(";
        write_to(out, T2{}, values);
        if constexpr (exponent_has_subterms) out << ")";
    }
};

}  // namespace traits

//! \} group Operators

}  // namespace xp
