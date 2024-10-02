// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Symbols
 * \brief Data structures to represent symbols from which expressions can be constructed.
 */
#pragma once

#include <utility>

#include <adac/utils.hpp>
#include <adac/dtype.hpp>
#include <adac/bindings.hpp>
#include <adac/eval.hpp>


namespace adac {

//! \addtogroup Symbols
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

//! Base class for negatable symbols
struct negatable {
    template<typename Self>
    constexpr auto operator-(this Self&& self) {
        return val<-1>*std::forward<Self>(self);
    }
};

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
struct symbol_value {
    template<typename... V>
        requires(bindings<V...>::template has_bindings_for<T>)
    static constexpr auto from(const bindings<V...>& bindings) noexcept {
        return bindings[T{}];
    }
};

template<typename T, auto _> struct value_of<var<T, _>> : symbol_value<var<T, _>> {};
template<typename T, auto _> struct value_of<let<T, _>> : symbol_value<let<T, _>> {};
template<auto v>
struct value_of<value<v>> {
    template<typename... T>
    static constexpr auto from(const bindings<T...>&) noexcept {
        return v;
    }
};

template<typename T>
struct symbol_derivative {
    template<typename V>
    static constexpr auto wrt(const type_list<V>& var) noexcept {
        if constexpr (concepts::same_remove_cvref_t_as<T, V>)
            return val<1>;
        else
            return val<0>;
    }
};

template<typename T, auto _> struct derivative_of<var<T, _>> : symbol_derivative<var<T, _>> {};
template<typename T, auto _> struct derivative_of<let<T, _>> : symbol_derivative<let<T, _>> {};
template<auto v>
struct derivative_of<value<v>> {
    template<typename V>
    static constexpr auto wrt(const bindings<V>&) noexcept {
        return val<0>;
    }
};

}  // namespace traits

//! \} group Symbols

}  // namespace adac
