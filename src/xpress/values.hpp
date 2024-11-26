// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Values
 * \brief Data structures to represent values that can be used in expressions.
 */
#pragma once

#include <utility>
#include <type_traits>

#include "utils.hpp"
#include "bindings.hpp"
#include "traits.hpp"


namespace xp {

//! \addtogroup Values
//! \{

//! symbol that represents a constant value
template<auto v>
struct value {
    template<typename Self> constexpr auto operator-(this Self&&) { return value<-v>{}; }
    template<typename Self, auto k> constexpr auto operator+(this Self&&, value<k>) { return value<v+k>{}; }
    template<typename Self, auto k> constexpr auto operator-(this Self&&, value<k>) { return value<v-k>{}; }
    template<typename Self, auto k> constexpr auto operator*(this Self&&, value<k>) { return value<v*k>{}; }
    template<typename Self, auto k> constexpr auto operator/(this Self&&, value<k>) { return value<v/k>{}; }
};

//! Instance of a constant value
template<auto v>
inline constexpr value<v> val;


namespace traits {

template<auto v>
struct is_unit_value<value<v>> : std::bool_constant<v == 1> {};

template<auto v>
struct is_zero_value<value<v>> : std::bool_constant<v == 0> {};

template<auto v>
struct nodes_of<value<v>> : std::type_identity<type_list<value<v>>> {};

template<auto v>
struct dtype_of<value<v>> : std::type_identity<decltype(v)> {};

template<auto v>
struct value_of<value<v>> {
    template<typename... T>
    static constexpr auto from(const bindings<T...>&) noexcept {
        return v;
    }
};

template<auto v>
struct derivative_of<value<v>> {
    template<typename V>
    static constexpr auto wrt(const type_list<V>&) noexcept {
        return val<0>;
    }
};

template<auto v>
struct stream<value<v>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>&) {
        out << v;
    }
};

}  // namespace traits

//! \} group Values

}  // namespace xp
