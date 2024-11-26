// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Operators
 * \brief Defines subtraction operations on expressions.
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
struct subtraction_of;

//! (Default) specialization for tensors
template<tensorial T1, tensorial T2>
    requires(shape_of_t<T1>{} == shape_of_t<T2>{})
struct subtraction_of<T1, T2> {
    template<same_remove_cvref_t_as<T1> _T1, same_remove_cvref_t_as<T2> _T2>
    constexpr auto operator()(_T1&& A, _T2&& B) const noexcept {
        using scalar = std::common_type_t<scalar_type_t<T1>, scalar_type_t<T2>>;
        using shape = shape_of_t<T1>;
        linalg::tensor<scalar, shape> result{};
        visit_indices_in(shape{}, [&] (const auto& idx) {
            result[idx] = access<T1>::at(idx, A) - access<T2>::at(idx, B);
        });
        return result;
    }
};

}  // namespace traits

struct subtract : operator_base<traits::subtraction_of, std::minus<void>> {};

}  // namespace operators

template<expression A, expression B>
    requires( not requires(const A& a, const B& b) { { a.operator-(b) }; } )
inline constexpr auto operator-(const A&, const B&) noexcept {
    if constexpr (traits::is_zero_value_v<A>)
        return -B{};
    else if constexpr (traits::is_zero_value_v<B>)
        return A{};
    else if constexpr (std::is_same_v<A, B>)
        return val<0>;
    else
        return operation<operators::subtract, A, B>{};
}

namespace traits {

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

}  // namespace traits

//! \} group Operators

}  // namespace xp
