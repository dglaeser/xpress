// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Operators
 * \brief Defines multiplication operations on expressions.
 */
#pragma once

#include <functional>

#include "../values.hpp"
#include "../expressions.hpp"
#include "common.hpp"


namespace xp {

//! \addtogroup Operators
//! \{

namespace operators {

namespace traits { template<typename A, typename B> struct multiplication_of; }

struct multiply : operator_base<traits::multiplication_of, std::multiplies<void>> {};

namespace traits { template<> struct is_commutative<multiply> : std::true_type {}; }

}  // namespace operators

template<expression A, expression B>
    requires( not requires(const A& a, const B& b) { { a.operator*(b) }; } )
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

namespace traits {

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

}  // namespace traits

//! \} group Operators

}  // namespace xp
