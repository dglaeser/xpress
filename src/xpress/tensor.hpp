// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Symbols
 * \brief Data structures to represent tensorial/vectorial symbols.
 */
#pragma once

#include <tuple>
#include <ostream>

#include "dtype.hpp"
#include "utils.hpp"
#include "traits.hpp"
#include "expressions.hpp"
#include "linalg.hpp"
#include "values.hpp"


namespace xp {

//! \addtogroup Symbols
//! \{

template<typename tensor, std::size_t... indices>
struct tensor_var : negatable {
    using dtype = typename tensor::dtype;
};


template<typename T = dtype::any, auto _ = [] () {}, std::size_t... dims> requires(sizeof...(dims) > 0)
struct tensor : bindable<T>, negatable {
    static constexpr md_shape<dims...> shape{};

    using bindable<T>::operator=;

    constexpr tensor() = default;
    constexpr tensor(md_shape<dims...>) noexcept {}

    template<std::size_t... i>
    constexpr auto operator[](const md_index<i...>&) const noexcept {
        static_assert(md_index<i...>{}.is_contained_in(shape), "Given index is not contained in this tensor's shape.");
        return tensor_var<tensor<T, _, dims...>, i...>{};
    }

    // for better compiler error messages about symbols being unique (not copyable)
    template<typename _T, auto __>
    constexpr tensor& operator=(const tensor<_T, __>&) = delete;
};

template<auto _ = [] () {}, std::size_t... dims>
tensor(const md_shape<dims...>&) -> tensor<dtype::any, _, dims...>;

template<std::size_t dim, typename T = dtype::any, auto _ = [] () {}>
using vector = tensor<T, _, dim>;


template<typename shape, expression... E>
    requires(shape::count == sizeof...(E) and shape::count > 0)
struct tensor_expression {
    using dtype = traits::common_dtype_t<traits::dtype_of_t<E>...>;

    constexpr tensor_expression() = default;
    constexpr tensor_expression(const shape&, const E&...) noexcept {}

    template<std::size_t... i>
    constexpr auto operator[](const md_index<i...>&) const noexcept {
        return std::get<md_index<i...>::as_flat_index_in(shape{})>(std::tuple<E...>{});
    }

    template<std::size_t i> requires(shape::dimensions == 1)
    constexpr auto operator[](const index_constant<i>&) const noexcept {
        return (*this)[md_ic<i>];
    }
};


template<typename T, auto _, std::size_t... d>
struct shape_of<tensor<T, _, d...>> : std::remove_cvref<decltype(tensor<T, _, d...>::shape)> {};

template<typename shape, typename... E>
struct shape_of<tensor_expression<shape, E...>> : std::type_identity<shape> {};

struct vector_expression {
    template<expression... E>
    static constexpr auto from(const E&... expressions) noexcept {
        return tensor_expression{shape<sizeof...(E)>, expressions...};
    }
};

template<typename shape, typename... T>
    requires(shape::count > 0 and (sizeof...(T) == 0 or sizeof...(T) == shape::count))
struct tensor_expression_builder {
    constexpr tensor_expression_builder() = default;
    constexpr tensor_expression_builder(const shape&) noexcept {};

    constexpr auto build() const noexcept {
        static_assert(sizeof...(T) > 0, "No entries of the tensor have been set yet.");
        static_assert(
            std::conjunction_v<traits::is_expression<T>...>,
            "All entries of the tensor have to be set to expressions before it can be built."
        );
        return tensor_expression<shape, T...>{};
    }

    template<expression E>
    constexpr auto filled_with(const E&) const noexcept {
        return _make_from(_repeat_n<shape::count, E>(type_list<>{}));
    }

    template<expression E, std::size_t... i>
    constexpr auto with(const E& expression, const md_index<i...>& idx) const noexcept {
        if constexpr (sizeof...(T) == 0)
            return _with(expression, idx, _repeat_n<shape::count, none>(type_list<>{}));
        else
            return _with(expression, idx, type_list<T...>{});
    }

 private:
    template<std::size_t i, typename E, typename... C> requires(std::conjunction_v<std::is_same<T, E>...>)
    constexpr auto _repeat_n(const type_list<C...>& current) const noexcept {
        if constexpr (sizeof...(C) == i)
            return current;
        else
            return _repeat_n<i, E>(type_list<C..., E>{});
    }

    template<typename E, std::size_t... i, typename... Ts> requires(sizeof...(Ts) == shape::count)
    constexpr auto _with(const E&, const md_index<i...>&, const type_list<Ts...>&) const noexcept {
        static_assert(md_index<i...>{}.is_contained_in(shape{}), "Given index is not contained in the specified shape.");
        static constexpr auto flat_idx = md_index<i...>::as_flat_index_in(shape{}).value;
        using replaced_types = replaced_type_at<flat_idx, E, type_list<>, type_list<Ts...>>::type;

        static_assert(is_any_of_v<E, replaced_types>, "Adding the expression unexpectedly failed. Please file a bug report.");
        return _make_from(replaced_types{});
    }

    template<typename... Ts>
    constexpr auto _make_from(const type_list<Ts...>&) const noexcept {
        return tensor_expression_builder<shape, Ts...>{};
    }

    template<std::size_t i, typename E, typename before, typename after>
    struct replaced_type_at;
    template<std::size_t i, typename E, typename... before, typename after0, typename... after>
    struct replaced_type_at<i, E, type_list<before...>, type_list<after0, after...>>
     : replaced_type_at<
        i,
        E,
        std::conditional_t<
            sizeof...(before) == i,
            type_list<before..., E>,
            type_list<before..., after0>
        >,
        type_list<after...>
    > {};
    template<std::size_t i, typename E, typename... replaced>
    struct replaced_type_at<i, E, type_list<replaced...>, type_list<>> : std::type_identity<type_list<replaced...>> {};
};

template<typename shape>
tensor_expression_builder(const shape&) -> tensor_expression_builder<shape>;

template<std::size_t n>
using vector_expression_builder = tensor_expression_builder<md_shape<n>>;


namespace traits {

template<typename T, auto _, std::size_t... dims>
struct stream<tensor<T, _, dims...>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) {
        using self = tensor<T, _, dims...>;
        out << values[self{}];
    }
};

template<typename tensor, std::size_t... i>
struct stream<tensor_var<tensor, i...>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) {
        out << values[tensor{}] << "[" << xp::values<i...>{} << "]";
    }
};

template<typename shape, typename... E>
struct stream<tensor_expression<shape, E...>> {
    template<typename... V>
    static constexpr void to(std::ostream& out, const bindings<V...>& values) {
        out << "T" << shape{} << "(";
        _write_args_to(out, values, unique_leaf_nodes_of_t<tensor_expression<shape, E...>>{});
        out << ")";
    }

 private:
    template<typename... V, typename L0, typename... L>
    static constexpr void _write_args_to(std::ostream& out, const bindings<V...>& values, const type_list<L0, L...>&) {
        stream<L0>::to(out, values);
        (..., (out << ", ", stream<L>::to(out, values)));
    }

    template<typename... V>
    static constexpr void _write_args_to(std::ostream& out, const bindings<V...>& values, const type_list<>&) {}
};

template<typename T, auto _, std::size_t... dims>
struct value_of<tensor<T, _, dims...>> {
    template<typename... V>
    static constexpr decltype(auto) from(const bindings<V...>& values) {
        using self = tensor<T, _, dims...>;
        using bound_type = std::remove_cvref_t<decltype(values[self{}])>;
        static_assert(tensorial<bound_type>, "Value type bound to tensor does not implement the concept 'tensorial'");
        return values[self{}];
    }
};

template<typename tensor, std::size_t... i>
struct value_of<tensor_var<tensor, i...>> {
    template<typename... V>
    static constexpr decltype(auto) from(const bindings<V...>& values) {
        using bound_tensor_type = std::remove_cvref_t<decltype(values[tensor{}])>;
        static_assert(tensorial<bound_tensor_type>, "Value type bound to tensor does not implement the concept 'tensorial'");
        return access<bound_tensor_type>::at(md_index<i...>{}, values[tensor{}]);
    }
};

template<typename shape, typename... E>
struct value_of<tensor_expression<shape, E...>> {
    template<typename... V>
    static constexpr decltype(auto) from(const bindings<V...>& values) {
        return linalg::tensor{shape{}, xp::value_of(E{}, values)...};
    }
};

template<typename T, auto _, std::size_t... dims>
struct nodes_of<tensor<T, _, dims...>> {
    using type = type_list<tensor<T, _, dims...>>;
};

template<typename tensor, std::size_t... i>
struct nodes_of<tensor_var<tensor, i...>> {
    using type = type_list<tensor_var<tensor, i...>>;
};

template<typename shape, typename... E>
struct nodes_of<tensor_expression<shape, E...>> {
    using type = merged_t<type_list<tensor_expression<shape, E...>>, merged_nodes_of_t<E...>>;
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

    // derivative wrt an entry of this tensor
    template<std::size_t... i>
    static constexpr decltype(auto) wrt(const type_list<tensor_var<tensor<T, _, dims...>, i...>>&) {
        return derivative_of<tensor_var<tensor<T, _, dims...>, i...>>::wrt(type_list<tensor<T, _, dims...>>{});
    }
};

template<typename tensor, std::size_t... i>
struct derivative_of<tensor_var<tensor, i...>> {
    template<typename V>
    static constexpr decltype(auto) wrt(const type_list<V>&) {
        if constexpr (std::is_same_v<V, tensor_var<tensor, i...>>)
            return val<1>;
        else if constexpr (std::is_same_v<V, tensor>)
            return tensor_expression_builder{tensor::shape}.filled_with(val<0>).with(val<1>, at<i...>()).build();
        else
            return val<0>;
    }
};

template<typename shape, typename... E>
struct derivative_of<tensor_expression<shape, E...>> {
    template<typename V>
    static constexpr decltype(auto) wrt(const type_list<V>& var) {
        return tensor_expression{shape{}, xp::derivative_of(E{}, var)...};
    }
};

}  // namespace traits

//! \} group Symbols

}  // namespace xp
