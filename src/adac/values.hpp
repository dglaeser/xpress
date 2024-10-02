// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Values
 * \brief Data structures to represent values that can be used in expressions.
 */
#pragma once

#include <utility>

#include <adac/utils.hpp>
#include <adac/dtype.hpp>
#include <adac/bindings.hpp>
#include <adac/eval.hpp>


namespace adac {

//! \addtogroup Values
//! \{

//! symbol that represents a constant value
template<auto v>
struct value {
    template<typename Self>
    constexpr auto operator-(this Self&& self) {
        return value<-v>{};
    }
};

//! instance of a constant value
template<auto v>
inline constexpr value<v> val;


namespace traits {

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

}  // namespace traits

//! \} group Values

}  // namespace adac
