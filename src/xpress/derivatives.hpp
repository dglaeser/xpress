// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Expressions
 * \brief Data structure to store derivatives of expressions wrt multiple variables.
 */
#pragma once

#include <type_traits>

#include "utils.hpp"
#include "bindings.hpp"


namespace xp {

//! \addtogroup Expressions
//! \{

template<typename E, typename V>
struct derivative {
    using expression = E;
    using variable = V;

    constexpr derivative() = default;
    constexpr derivative(const E&, const V&) noexcept {}

    constexpr E get() const noexcept requires(std::is_default_constructible_v<E>) {
        return E{};
    }
};

template<typename E, typename V>
derivative(E&&, V&&) -> derivative<std::remove_cvref_t<E>, std::remove_cvref_t<V>>;


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct is_derivative : std::false_type {};

    template<typename E, typename V>
    struct is_derivative<derivative<E, V>> : std::true_type {};

}  // namespace detail
#endif  // DOXYGEN

template<typename... D>
    requires(std::conjunction_v<detail::is_derivative<D>...>)
struct derivatives : private indexed<typename D::variable...> {
    constexpr derivatives(const D&...) noexcept {}

    //! Return the derivative w.r.t. the given variable
    template<typename V>
        requires(is_any_of_v<V, typename D::variable...>)
    constexpr auto wrt(const V& var) const noexcept {
        return indexed<D...>{}.get(this->index_of(var)).get();
    }

    //! Evaluate the derivatives at the given values
    template<concepts::binder... V>
    constexpr auto at(V&&... values) const noexcept {
        return at(bindings{std::forward<V>(values)...});
    }

    //! Evaluate the derivatives at the given value bindings
    template<typename... V>
    constexpr auto at(const bindings<V...>& values) const noexcept {
        return bindings{_binder_for(D{}, values)...};
    }

 private:
    template<typename E, typename V, typename... Vs>
        requires(concepts::evaluatable_with<E, Vs...>)
    constexpr auto _binder_for(const derivative<E, V>&, const bindings<Vs...>& values) const noexcept {
        return value_binder{V{}, traits::value_of<E>::from(values)};
    }
};

template<typename... D>
derivatives(D&&...) -> derivatives<std::remove_cvref_t<D>...>;

//! \} group Expressions

}  // namespace xp