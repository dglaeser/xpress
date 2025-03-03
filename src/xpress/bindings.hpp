// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Bindings
 * \brief Data structures for binding values to expressions.
 */
#pragma once

#include <type_traits>

#include "utils.hpp"
#include "dtype.hpp"


namespace xp {

//! \addtogroup Bindings
//! \{

//! Data structure to store a value bound to a symbol
template<typename S, typename V>
struct value_binder {
    using symbol_type = std::remove_cvref_t<S>;
    using value_type = std::remove_cvref_t<V>;

    template<same_remove_cvref_t_as<V> _V>
    constexpr value_binder(const S&, _V&& v) noexcept
    : _value{std::forward<_V>(v)}
    {}

    //! Return the bound value
    template<typename Self>
    constexpr decltype(auto) get(this Self&& self) noexcept {
        if constexpr (!std::is_lvalue_reference_v<Self>)
            return std::move(self._value).get();
        else
            return self._value.get();
    }

 private:
    value_or_reference<V> _value;
};

template<typename S, typename V>
value_binder(S&&, V&&) -> value_binder<std::remove_cvref_t<S>, V>;

//! Concept for types that bind values to symbols
template<typename T>
concept binder = requires(const T& t) {
    typename T::symbol_type;
    typename T::value_type;
    { t.get() } -> same_remove_cvref_t_as<typename T::value_type>;
};

//! Data structure to store values bound to symbols.
template<binder... B>
    requires(are_unique_v<typename B::symbol_type...>)
struct bindings : private indexed_tuple<B...> {
 private:
    using base = indexed_tuple<B...>;

    template<typename T>
    using symbol_type_of = typename std::remove_cvref_t<T>::symbol_type;

    template<typename T, typename... Bs>
    struct binder_type_for;
    template<typename T>
    struct binder_type_for<T> : std::type_identity<void> {};
    template<typename T, typename B0, typename... Bs>
    struct binder_type_for<T, B0, Bs...> {
        static constexpr bool is_same_symbol = std::is_same_v<symbol_type_of<B0>, T>;
        using type = std::conditional_t<is_same_symbol, B0, typename binder_type_for<T, Bs...>::type>;
    };

    template<typename T>
    static constexpr bool is_bound = is_any_of_v<std::remove_cvref_t<T>, symbol_type_of<B>...>;

    template<typename T> requires(sizeof...(B) > 0 and is_bound<T>)
    using binder_type = binder_type_for<std::remove_cvref_t<T>, B...>::type;

 public:
    template<typename T>
    static constexpr bool has_bindings_for = is_bound<T>;

    constexpr bindings(B... binders) noexcept
    : base(std::forward<B>(binders)...)
    {}

    //! Return the value bound to the given symbol
    template<typename S, typename T> requires(has_bindings_for<T>)
    constexpr decltype(auto) operator[](this S&& self, const T&) noexcept {
        return std::forward<S>(self).get(self.template index_of<binder_type<T>>()).get();
    }

    //! Concatenate these bindings with the given ones
    template<binder... Bs>
        requires(are_unique_v<typename Bs::symbol_type..., typename B::symbol_type...>)
    constexpr auto concatenated_with(bindings<Bs...>&& other) && noexcept {
        return xp::bindings{
            this->get(this->template index_of<B>())...,
            value_binder{typename Bs::symbol_type{}, std::move(other)[typename Bs::symbol_type{}]}...
        };
    }

    //! Concatenation operator
    template<binder... Bs>
    friend constexpr auto operator&(bindings&& left, bindings<Bs...>&& right) noexcept {
        return std::move(left).concatenated_with(std::move(right));
    }

    //! Iterate over all symbols and their bound values
    template<typename V>
    friend constexpr void for_each(const bindings& bindings, V&& visitor) {
        static_assert(std::conjunction_v<
            std::is_invocable<V, typename B::symbol_type, const typename B::value_type>...
        >, "visitor must have the signature visitor(const auto& symbol, const auto& bound_value)");
        (..., visitor(typename B::symbol_type{}, bindings[typename B::symbol_type{}]));
    }
};

//! Specialization for a zero number of symbols (for compatibility purposes)
template<>
struct bindings<> {
    template<typename... T>
    static constexpr bool has_bindings_for = false;
};

template<typename... B>
bindings(B&&...) -> bindings<B...>;

//! Create bindings from the given bound symbols
template<binder... B>
inline constexpr auto at(B&&... b) {
    return bindings{std::forward<B>(b)...};
}

//! Alternative syntax for creating bindings
template<binder... B>
inline constexpr auto with(B&&... b) {
    return bindings{std::forward<B>(b)...};
}

//! \} group Bindings

}  // namespace xp
