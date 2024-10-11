// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Symbols
 * \brief Data structures to represent symbols from which expressions can be constructed.
 */
#pragma once

#include <utility>
#include <type_traits>

#include "utils.hpp"
#include "dtype.hpp"
#include "expressions.hpp"


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

template<typename T, auto _> struct is_variable<var<T, _>> : std::true_type {};

template<typename T, auto _> struct is_symbol<var<T, _>> : std::true_type {};
template<typename T, auto _> struct is_symbol<let<T, _>> : std::true_type {};

template<typename T, auto _> struct nodes_of<var<T, _>> : std::type_identity<type_list<var<T, _>>> {};
template<typename T, auto _> struct nodes_of<let<T, _>> : std::type_identity<type_list<let<T, _>>> {};


template<typename T>
struct _symbol_value {
    template<typename... V>
        requires(bindings<V...>::template has_bindings_for<T>)
    static constexpr decltype(auto) from(const bindings<V...>& bindings) noexcept {
        // TODO: necessary?
        using bound_type = std::remove_cvref_t<decltype(bindings[T{}])>;
        static_assert(traits::is_scalar_v<bound_type>, "Symbol values have to be scalars");
        return bindings[T{}];
    }
};

template<typename T, auto _> struct value_of<var<T, _>> : _symbol_value<var<T, _>> {};
template<typename T, auto _> struct value_of<let<T, _>> : _symbol_value<let<T, _>> {};


template<typename T>
struct _symbol_derivative {
    template<typename V>
    static constexpr auto wrt(const type_list<V>&) noexcept {
        if constexpr (concepts::same_remove_cvref_t_as<T, V>)
            return val<1>;
        else
            return val<0>;
    }
};

template<typename T, auto _> struct derivative_of<var<T, _>> : _symbol_derivative<var<T, _>> {};
template<typename T, auto _> struct derivative_of<let<T, _>> : _symbol_derivative<let<T, _>> {};


template<typename T>
struct _bound_symbol_stream {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) {
        out << values[T{}];
    }
};

template<typename T, auto _> struct stream<var<T, _>> : _bound_symbol_stream<var<T, _>> {};
template<typename T, auto _> struct stream<let<T, _>> : _bound_symbol_stream<let<T, _>> {};

}  // namespace traits

//! \} group Symbols

}  // namespace adac
