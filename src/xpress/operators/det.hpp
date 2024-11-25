// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Operators
 * \brief Defines the determinant operator on tensorial expressions.
 */
#pragma once

#include <functional>

#include "../values.hpp"
#include "../expressions.hpp"
#include "../linalg.hpp"
#include "../tensor.hpp"
#include "common.hpp"


namespace xp {

//! \addtogroup Operators
//! \{

namespace operators {

namespace traits { template<typename T> struct determinant_of; }

struct default_determinant_operator {
    template<tensorial T>
    constexpr auto operator()(T&& t) const noexcept {
        return linalg::determinant_of(std::forward<T>(t));
    }
};

struct determinant : operator_base<traits::determinant_of, default_determinant_operator> {};

}  // namespace operators

template<tensorial_expression T>
inline constexpr auto det(const T&) noexcept {
    static_assert(shape_of_t<T>{}.is_square, "Determinant can only be taken on square matrices.");
    return operation<operators::determinant, T>{};
}

namespace traits {

template<tensorial_expression T>
struct derivative_of<operation<operators::determinant, T>> {
    static constexpr auto t_shape = shape_of_t<T>{};
    static_assert(t_shape.is_square, "Determinant derivative can only be computed for square matrices.");
    static_assert(t_shape.first() == 2 || t_shape.first() == 3, "Determinant derivative is only implemented for 2d & 3d matrices.");
    static_assert(t_shape.last() == 2 || t_shape.last(), "Determinant derivative is only implemented for 2d & 3d matrices.");

    template<typename V>
    static constexpr decltype(auto) wrt(const type_list<V>&) {
        if constexpr (std::is_same_v<V, T>) {
            if constexpr (t_shape.first() == 2) {
                constexpr auto a = T{}[at<0, 0>()]; constexpr auto b = T{}[at<0, 1>()];
                constexpr auto c = T{}[at<1, 0>()]; constexpr auto d = T{}[at<1, 1>()];
                return tensor_expression{shape<2, 2>, d, -c, -b, a};
            } else {
                constexpr auto a = T{}[at<0, 0>()]; constexpr auto b = T{}[at<0, 1>()]; constexpr auto c = T{}[at<0, 2>()];
                constexpr auto d = T{}[at<1, 0>()]; constexpr auto e = T{}[at<1, 1>()]; constexpr auto f = T{}[at<1, 2>()];
                constexpr auto g = T{}[at<2, 0>()]; constexpr auto h = T{}[at<2, 1>()]; constexpr auto i = T{}[at<2, 2>()];
                return tensor_expression{shape<3, 3>,
                    e*i - f*h, f*g - d*i, d*h - e*g,
                    c*h - b*i, a*i - c*g, b*g - a*h,
                    b*f - c*e, c*d - a*f, a*e - b*d
                };
            }
        } else {
            return val<0>;
        }
    }
};

template<tensorial_expression T>
struct stream<operation<operators::determinant, T>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) noexcept {
        out << "det("; write_to(out, T{}, values); out << ")";
    }
};

}  // namespace traits

//! \} group Operators

}  // namespace xp
