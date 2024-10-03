// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Values
 * \brief Data structures to represent values that can be used in expressions.
 */
#pragma once

#include <utility>
#include <type_traits>

#include <adac/utils.hpp>
#include <adac/dtype.hpp>
#include <adac/bindings.hpp>
#include <adac/expression_traits.hpp>


namespace adac {

//! \addtogroup Values
//! \{

//! symbol that represents a constant value
template<auto v>
struct value {
    template<typename Self> constexpr auto operator-(this Self&& self) { return value<-v>{}; }
    template<typename Self, auto k> constexpr auto operator+(this Self&& self, value<k>) { return value<v+k>{}; }
    template<typename Self, auto k> constexpr auto operator-(this Self&& self, value<k>) { return value<v-k>{}; }
    template<typename Self, auto k> constexpr auto operator*(this Self&& self, value<k>) { return value<v*k>{}; }
    template<typename Self, auto k> constexpr auto operator/(this Self&& self, value<k>) { return value<v/k>{}; }
};

//! instance of a constant value
template<auto v>
inline constexpr value<v> val;


namespace traits {

template<auto v, auto k>
struct disable_generic_arithmetic_operators<value<v>, value<k>> : std::true_type {};

template<auto v>
struct nodes_of<value<v>> : std::type_identity<type_list<value<v>>> {};

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
    static constexpr auto wrt(const bindings<V>&) noexcept {
        return val<0>;
    }
};

template<auto v>
struct dtype_of<value<v>> {
    using type = std::conditional_t<
        std::is_floating_point_v<decltype(v)>,
        dtype::real,
        std::conditional_t<
            std::is_integral_v<decltype(v)>,
            dtype::integral,
            dtype::any
        >
    >;
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

}  // namespace adac
