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
#include "traits.hpp"
#include "expressions.hpp"
#include "linalg.hpp"
#include "values.hpp"


namespace adac {

//! \addtogroup Symbols
//! \{


template<typename T = dtype::any, auto _ = [] () {}, std::size_t... dims>
struct tensor : bindable<T>, negatable {
    using bindable<T>::operator=;

    constexpr tensor() = default;
    constexpr tensor(md_shape<dims...>) noexcept {}

    // for better compiler error messages about symbols being unique (not copyable)
    template<typename _T, auto __>
    constexpr tensor& operator=(const tensor<_T, __>&) = delete;
};

template<auto _ = [] () {}, std::size_t... dims>
tensor(const md_shape<dims...>&) -> tensor<dtype::any, _, dims...>;


template<std::size_t dim, typename T = dtype::any, auto _ = [] () {}>
using vector = tensor<T, _, dim>;


template<typename shape, concepts::expression... E> requires(shape::count == sizeof...(E))
struct tensor_expression : indexed<E...> {
    constexpr tensor_expression(const shape&, const E&...) noexcept {}

    template<std::size_t... i>
    constexpr auto operator[](const md_index<i...>& idx) const noexcept {
        return this->get(i_c<md_index<i...>::as_flat_index_in(shape{})>);
    }
};


namespace traits {

template<typename T, auto _, std::size_t... dims>
struct stream<tensor<T, _, dims...>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) {
        using self = tensor<T, _, dims...>;
        out << values[self{}];
    }
};

template<typename T, auto _, std::size_t... dims>
struct value_of<tensor<T, _, dims...>> {
    template<typename... V>
    static constexpr decltype(auto) from(const bindings<V...>& values) {
        using self = tensor<T, _, dims...>;
        using bound_type = std::remove_cvref_t<decltype(values[self{}])>;
        static_assert(
            linalg::concepts::tensor<bound_type>,
            "Value type bound to tensor does not implement the concept 'linalg::concepts::tensor'"
        );
        return values[self{}];
    }
};

template<typename shape, typename... E>
struct value_of<tensor_expression<shape, E...>> {
    template<typename... V>
    static constexpr decltype(auto) from(const bindings<V...>& values) {
        return linalg::tensor{shape{}, adac::value_of(E{}, values)...};
    }
};

template<typename T, auto _, std::size_t... dims>
struct nodes_of<tensor<T, _, dims...>> {
    using type = type_list<tensor<T, _, dims...>>;
};

template<typename shape, typename... E>
struct nodes_of<tensor_expression<shape, E...>> {
    using type = merged_types_t<type_list<tensor_expression<shape, E...>>, merged_nodes_of_t<E...>>;
};

template<typename T, auto _, std::size_t... dims>
struct derivative_of<tensor<T, _, dims...>> {
    template<typename V>
    static constexpr decltype(auto) wrt(const type_list<V>&) {
        if constexpr (std::is_same_v<V, tensor<T, _, dims...>>)
            return val<1>;
        else
            return val<0>;
    }
};

template<typename shape, typename... E>
struct derivative_of<tensor_expression<shape, E...>> {
    template<typename V>
    static constexpr decltype(auto) wrt(const type_list<V>&) {
        if constexpr (std::is_same_v<V, tensor_expression<shape, E...>>)
            return val<1>;
        else
            return val<0>;
    }
};

}  // namespace traits

//! \} group Symbols

}  // namespace adac
