// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Operators
 * \brief Defines division operations on expressions.
 */
#pragma once

#include <functional>

#include "../values.hpp"
#include "../expressions.hpp"
#include "../linalg.hpp"
#include "common.hpp"


namespace xp {

//! \addtogroup Operators
//! \{

namespace operators {

namespace traits {

template<typename A, typename B>
struct division_of;

//! (Default) specialization for tensors with scalars
template<tensorial T, typename S> requires(is_scalar_v<S>)
struct division_of<T, S> {
    template<same_remove_cvref_t_as<T> _T, same_remove_cvref_t_as<S> _S>
    constexpr T operator()(_T&& tensor, _S&& scalar) const noexcept {
        T result;
        visit_indices_in(shape_of_t<T>{}, [&] (const auto& idx) {
            scalar_type_t<T>& value_at_idx = access<T>::at(idx, result);
            value_at_idx = access<T>::at(idx, tensor)/scalar;
        });
        return result;
    }
};

}  // namespace traits

struct divide : operator_base<traits::division_of, std::divides<void>> {};

}  // namespace operators

template<expression A, expression B>
    requires( not requires(const A& a, const B& b) { { a.operator/(b) }; } )
inline constexpr auto operator/(const A&, const B&) noexcept {
    static_assert(!traits::is_zero_value_v<B>, "Attempted division by zero.");
    if constexpr (traits::is_zero_value_v<A>)
        return val<0>;
    else if constexpr (traits::is_unit_value_v<B>)
        return A{};
    else if constexpr (std::is_same_v<A, B>)
        return val<1>;
    else
        return operation<operators::divide, A, B>{};
}

namespace traits {

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

}  // namespace traits

//! \} group Operators

}  // namespace xp
