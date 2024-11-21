// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Operators
 * \brief Defines natural logarithm operations on expressions.
 */
#pragma once

#include <cmath>

#include "../values.hpp"
#include "../expressions.hpp"
#include "common.hpp"


namespace xp {

//! \addtogroup Operators
//! \{

namespace operators {

namespace traits { template<typename A> struct log_of; }

struct default_log_operator {
    template<typename A>
    constexpr auto operator()(A&& a) const noexcept {
        return std::log(std::forward<A>(a));
    }
};

struct log : operator_base<traits::log_of, default_log_operator> {};

}  // namespace operators

template<concepts::expression A>
inline constexpr auto log(const A&) noexcept {
    static_assert(!traits::is_zero_value_v<A>, "Logarithm of zero is not defined.");
    if constexpr (traits::is_unit_value_v<A>)
        return val<1>;
    else
        return operation<operators::log, A>{};
}

namespace traits {

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

}  // namespace traits

//! \} group Operators

}  // namespace xp
