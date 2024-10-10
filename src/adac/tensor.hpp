// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Symbols
 * \brief Data structures to represent tensorial/vectorial symbols.
 */
#pragma once

#include <ostream>

#include "utils.hpp"
#include "expressions.hpp"
#include "linalg.hpp"


namespace adac {

//! \addtogroup Symbols
//! \{


template<typename T = dtype::any, auto _ = [] () {}, std::size_t... dims>
struct tensor : bindable<T> {
    using bindable<T>::operator=;

    constexpr tensor() = default;
    constexpr tensor(md_shape<dims...>) noexcept {}

    // for better compiler error messages about symbols being unique (not copyable)
    template<typename _T, auto __>
    constexpr tensor& operator=(const tensor<_T, __>&) = delete;
};

template<auto _ = [] () {}, std::size_t... dims>
tensor(const md_shape<dims...>&) -> tensor<dtype::any, _, dims...>;


namespace traits {

template<typename T, auto _, std::size_t... dims>
struct stream<tensor<T, _, dims...>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) {
        using self = tensor<T, _, dims...>;
        out << values[self{}];
    }
};

}  // namespace traits

//! \} group Symbols

}  // namespace adac
