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
#include "../linalg.hpp"
#include "common.hpp"


namespace xp {

//! \addtogroup Operators
//! \{

namespace operators {

namespace traits {

template<typename A, typename B>
struct multiplication_of;

//! (Default) specialization for tensors with scalars
template<tensorial T, typename S> requires(is_scalar_v<S>)
struct multiplication_of<T, S> {
    template<same_remove_cvref_t_as<T> _T, same_remove_cvref_t_as<S> _S>
    constexpr T operator()(_T&& tensor, _S&& scalar) const noexcept {
        T result;
        visit_indices_in(shape_of_t<T>{}, [&] (const auto& idx) {
            scalar_type_t<T>& value_at_idx = access<T>::at(idx, result);
            value_at_idx = access<T>::at(idx, tensor)*scalar;
        });
        return result;
    }
};

//! (Default) specialization for scalars with tensors
template<typename S, tensorial T> requires(is_scalar_v<S>)
struct multiplication_of<S, T> {
    template<same_remove_cvref_t_as<S> _S, same_remove_cvref_t_as<T> _T>
    constexpr T operator()(_S&& scalar, _T&& tensor) const noexcept {
        return multiplication_of<T, S>{}(std::forward<_T>(tensor), std::forward<_S>(scalar));
    }
};

//! (Default) specialization for tensors with tensors
template<tensorial T1, tensorial T2>
    requires(shape_of_t<T1>{} == shape_of_t<T2>{})
struct multiplication_of<T1, T2> {
    template<same_remove_cvref_t_as<T1> _T1, same_remove_cvref_t_as<T2> _T2>
    constexpr auto operator()(_T1&& A, _T2&& B) const noexcept {
        scalar_type_t<T1> result{0};
        visit_indices_in(shape_of_t<T1>{}, [&] (const auto& idx) {
            result += access<T1>::at(idx, A)*access<T2>::at(idx, B);
        });
        return result;
    }
};

}  // namespace traits

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
