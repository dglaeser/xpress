// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Symbols
 * \brief Data structures to represent symbols from which expressions can be constructed.
 */
#pragma once

#include <utility>

#include <adac/eval.hpp>
#include <adac/utils.hpp>
#include <adac/dtype.hpp>
#include <adac/expression_interfaces.hpp>


namespace adac {

//! \addtogroup Symbols
//! \{

//! Symbol to represent an independent variable
template<typename T = dtype::any, auto = [] () {}>
struct var : negatable, bindable<T> {
    using bindable<T>::operator=;

    // for better compiler error messages about symbols being unique (not copyable)
    template<typename _T, auto __>
    constexpr var& operator=(const var<_T, __>&) = delete;
};

//! Symbol to represent a parameter
template<typename T = dtype::any, auto = [] () {}>
struct let : negatable, bindable<T> {
    using bindable<T>::operator=;

    // for better compiler error messages about symbols being unique (not copyable)
    template<typename _T, auto __>
    constexpr let& operator=(const let<_T, __>&) = delete;
};


namespace traits {

template<typename T>
struct _symbol_value {
    template<typename... V>
        requires(bindings<V...>::template has_bindings_for<T>)
    static constexpr auto from(const bindings<V...>& bindings) noexcept {
        return bindings[T{}];
    }
};

template<typename T, auto _> struct value_of<var<T, _>> : _symbol_value<var<T, _>> {};
template<typename T, auto _> struct value_of<let<T, _>> : _symbol_value<let<T, _>> {};

template<typename T>
struct _symbol_derivative {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& var) noexcept {
        if constexpr (concepts::same_remove_cvref_t_as<T, V>)
            return val<1>;
        else
            return val<0>;
    }
};

template<typename T, auto _> struct derivative_of<var<T, _>> : _symbol_derivative<var<T, _>> {};
template<typename T, auto _> struct derivative_of<let<T, _>> : _symbol_derivative<let<T, _>> {};

}  // namespace traits

//! \} group Symbols

}  // namespace adac
