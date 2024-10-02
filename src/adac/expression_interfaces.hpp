// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Expressions
 * \brief Base classes that expression implementations can derive from.
 */
#pragma once

#include <utility>

#include <adac/dtype.hpp>
#include <adac/bindings.hpp>
#include <adac/values.hpp>


namespace adac {

//! \addtogroup Expressions
//! \{

//! Base class for negatable symbols
struct negatable {
    template<typename Self>
    constexpr auto operator-(this Self&& self) {
        return val<-1>*std::forward<Self>(self);
    }
};

//! base class for bindable symbols/expressions
template<concepts::dtype T = dtype::any>
struct bindable {
    using dtype = T;

    //! bind the given value to this symbol
    template<typename Self, typename V> requires(concepts::accepts<T, V>)
    constexpr auto operator=(this Self&& self, V&& value) noexcept {
        return value_binder(std::forward<Self>(self), std::forward<V>(value));
    }
};

//! \} group Expressions

}  // namespace adac
