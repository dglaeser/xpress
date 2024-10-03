// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Bindings
 * \brief Data structures for binding values to expressions.
 */
#pragma once

#include <type_traits>

#include <adac/utils.hpp>
#include <adac/dtype.hpp>

namespace adac {

//! \addtogroup Bindings
//! \{

//! Data structure to store a value bound to a symbol
template<typename S, typename V>
struct value_binder {
    using symbol_type = std::remove_cvref_t<S>;
    using value_type = std::remove_cvref_t<V>;

    template<concepts::same_remove_cvref_t_as<V> _V>
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


namespace concepts {

//! Concept for types that bind values to symbols
template<typename T>
concept binder = requires(const T& t) {
    typename T::symbol_type;
    typename T::value_type;
    { t.get() } -> same_remove_cvref_t_as<typename T::value_type>;
};

}  // namespace concepts


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct is_value_binder : std::bool_constant<concepts::binder<T>> {};

    template<typename... B>
    inline constexpr bool are_binders = std::conjunction_v<is_value_binder<B>...>;

}  // namespace detail
#endif  // DOXYGEN

//! Data structure to store values bound to symbols.
template<typename... B>
    requires(are_unique_v<B...> and detail::are_binders<B...>)
struct bindings : private variadic_accessor<B...> {
 private:
    using base = variadic_accessor<B...>;

    template<typename T>
    using symbol_type_of = typename std::remove_cvref_t<T>::symbol_type;

    template<typename T, typename... Bs>
    struct binder_type_for;

    template<typename T, typename B0, typename... Bs>
    struct binder_type_for<T, B0, Bs...> {
        using type = std::conditional_t<
            concepts::same_remove_cvref_t_as<T, symbol_type_of<B0>>,
            B0,
            typename binder_type_for<T, Bs...>::type
        >;
    };

    template<typename T>
    struct binder_type_for<T> {
        using type = void;
    };

    template<typename T>
    struct binds : std::disjunction<std::is_same<std::remove_cvref_t<T>, symbol_type_of<B>>...> {};

    template<typename T> requires(sizeof...(B) > 0 and binds<T>::value)
    using binder_type = binder_type_for<T, B...>::type;

 public:
    template<typename... T>
    static constexpr bool has_bindings_for = std::conjunction_v<binds<T>...>;

    using common_value_type = std::common_type_t<typename std::remove_cvref_t<B>::value_type...>;

    constexpr bindings(B... binders) noexcept
    : base(std::forward<B>(binders)...)
    {}

    //! Return the value bound to the symbol of type T
    template<typename T> requires(has_bindings_for<T>)
    constexpr decltype(auto) get() const noexcept {
        return base::get(this->template index_of<binder_type<T>>()).get();
    }

    //! Return the value bound to the given symbol
    template<typename T> requires(has_bindings_for<T>)
    constexpr decltype(auto) operator[](const T&) const noexcept {
        return this->template get<T>();
    }
};

//! Specialization for a zero number of symbols (for compatibility purposes)
template<>
struct bindings<> {
    template<typename... T>
    static constexpr bool has_bindings_for = false;

    using common_value_type = double;
};

template<typename... B>
bindings(B&&...) -> bindings<B...>;


//! Create bindings from the given bound symbols
template<typename... B> requires(detail::are_binders<B...>)
inline constexpr auto at(B&&... b) {
    return bindings{std::forward<B>(b)...};
}

//! \} group Bindings

}  // namespace adac
