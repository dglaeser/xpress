// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Operators
 * \brief Arithmetic operators on symbols/expressions.
 */
#pragma once

#include <type_traits>
#include <functional>

#include <adac/utils.hpp>
#include <adac/eval.hpp>
#include <adac/dtype.hpp>
#include <adac/expression_interfaces.hpp>


namespace adac {

//! \addtogroup Operators
//! \{

namespace operators {

struct add : std::plus<void> {};
struct subtract : std::minus<void> {};

}  // namespace operators

//! Class to represent expressions resulting from an operator applied to the given terms
template<typename op, concepts::expression T1, concepts::expression... Ts>
struct expression : bindable<dtype::common_dtype_of_t<T1, Ts...>>, negatable {
    constexpr expression() = default;
    constexpr expression(const op&, const Ts&...) noexcept {}
};

template<typename op, typename... Ts>
expression(op&&, Ts&&...) -> expression<std::remove_cvref_t<op>, std::remove_cvref_t<Ts>...>;


template<concepts::expression A, concepts::expression B>
    requires(!traits::disable_generic_arithmetic_operators<A, B>::value)
inline constexpr auto operator+(const A&, const B&) noexcept {
    return expression<operators::add, A, B>{};
}

template<concepts::expression A, concepts::expression B>
    requires(!traits::disable_generic_arithmetic_operators<A, B>::value)
inline constexpr auto operator-(const A&, const B&) noexcept {
    return expression<operators::subtract, A, B>{};
}

namespace traits {

template<typename op, typename... Ts>
struct dtype_of<expression<op, Ts...>> : std::type_identity<typename expression<op, Ts...>::dtype> {};

template<typename op, typename... Ts>
struct value_of<expression<op, Ts...>> {
    template<typename... V>
    static constexpr auto from(const bindings<V...>& bindings) noexcept {
        return op{}(evaluate(Ts{}, bindings)...);
    }
};

template<typename T1, typename T2>
struct derivative_of<expression<operators::add, T1, T2>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& vars) noexcept {
        return differentiate(T1{}, vars) + differentiate(T2{}, vars);
    }
};

template<typename T1, typename T2>
struct derivative_of<expression<operators::subtract, T1, T2>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& vars) noexcept {
        return differentiate(T1{}, vars) - differentiate(T2{}, vars);
    }
};

}  // namespace traits

//! \} group Operators

}  // namespace adac
