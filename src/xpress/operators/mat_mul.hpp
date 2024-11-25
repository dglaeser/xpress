// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Operators
 * \brief Defines an operator for matrix multiplication of tensorial expressions.
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

namespace traits { template<typename A, typename B> struct mat_mul_of; }

struct default_mat_mul_operator {
    template<tensorial T1, tensorial T2>
    constexpr auto operator()(T1&& t1, T2&& t2) const noexcept {
        return linalg::mat_mul(std::forward<T1>(t1), std::forward<T2>(t2));
    }
};

struct mat_mul : operator_base<traits::mat_mul_of, default_mat_mul_operator> {};

}  // namespace operators

template<tensorial_expression T1, tensorial_expression T2>
inline constexpr auto mat_mul(const T1&, const T2&) noexcept {
    return operation<operators::mat_mul, T1, T2>{};
}

namespace traits {

template<tensorial_expression T1, tensorial_expression T2>
struct derivative_of<operation<operators::mat_mul, T1, T2>> {
    template<typename V>
    static constexpr decltype(auto) wrt(const type_list<V>& var) {
        return _mat_mul(xp::detail::differentiate<T1>(var), T2{}) + _mat_mul(T1{}, xp::detail::differentiate<T2>(var));
    }

 private:
    template<tensorial_expression _T1, tensorial_expression _T2>
    static constexpr decltype(auto) _mat_mul(const _T1& t1, const _T2& t2) noexcept {
        return mat_mul(t1, t2);
    }

    template<expression _T1, expression _T2>
        requires(!tensorial_expression<_T1> or !tensorial_expression<_T2>)
    static constexpr decltype(auto) _mat_mul(const _T1& t1, const _T2& t2) noexcept {
        return t1*t2;
    }
};

// stream it the same way as multiplication
template<tensorial_expression T1, tensorial_expression T2>
struct stream<operation<operators::mat_mul, T1, T2>>
: stream<operation<operators::multiply, T1, T2>> {};

}  // namespace traits

//! \} group Operators

}  // namespace xp
