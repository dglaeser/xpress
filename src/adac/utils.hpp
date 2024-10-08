// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Utilities
 * \brief Utility classes.
 */

#pragma once

#include <ostream>
#include <type_traits>

#include <cpputils/type_traits.hpp>
#include <cpputils/utility.hpp>


namespace adac {

//! bring in all cpputils utility
using namespace cpputils;

//! Type to represent a compile-time index
template<std::size_t i>
struct index_constant {
    static constexpr std::size_t value = i;

    template<auto _i>
    constexpr bool operator==(const index_constant<_i>&) const noexcept {
        return _i == i;
    }
};

//! Instance of a compile-time index
template<std::size_t i>
inline constexpr index_constant<i> index{};


namespace traits {

template<auto a, auto b>
struct is_equal : std::bool_constant<(a == b)> {};

}  // namespace traits


#ifndef DOXYGEN
namespace detail {

    template<std::size_t i, auto v>
    struct value_i {
        static constexpr auto at(index_constant<i>) noexcept {
            return v;
        }
    };

    template<typename I, auto...>
    struct values;
    template<std::size_t... i, auto... v> requires(sizeof...(i) == sizeof...(v))
    struct values<std::index_sequence<i...>, v...> : value_i<i, v>... {
        using value_i<i, v>::at...;
    };

}  // namespace detail
#endif  // DOXYGEN

//! Class to represent a list of values.
template<auto... v>
struct value_list : detail::values<std::make_index_sequence<sizeof...(v)>, v...> {
    static constexpr std::size_t size = sizeof...(v);

    //! Return the value at the given index in the list
    template<std::size_t i> requires(i < size)
    static constexpr auto at(index_constant<i> idx) {
        using base = detail::values<std::make_index_sequence<size>, v...>;
        return base::at(idx);
    }

    //! Perform a reduction operation on this list
    template<typename op, typename T>
    static constexpr auto reduce_with(op&& action, T&& initial) {
        return _reduce_with(std::forward<op>(action), std::forward<T>(initial), v...);
    }

    //! Concatenate this list with another one
    template<auto... _v>
    constexpr auto operator+(const value_list<_v...>&) const {
        return value_list<v..., _v...>{};
    }

    //! Test this list for equality with another one
    template<auto... _v>
    constexpr bool operator==(const value_list<_v...>&) const {
        if constexpr (sizeof...(_v) == size)
            return std::conjunction_v<traits::is_equal<v, _v>...>;
        return false;
    }

    //! Write this list to the given output stream
    friend std::ostream& operator<<(std::ostream& s, const value_list&) {
        s << "[";
        (s << ... << ((v == at(index<0>) ? "" : ", ") + std::to_string(v)));
        s << "]";
        return s;
    }

 private:
    template<typename op, typename T>
    static constexpr auto _reduce_with(op&&, T&& initial) noexcept {
        return std::forward<T>(initial);
    }

    template<typename op, typename T, typename V0, typename... V>
    static constexpr auto _reduce_with(op&& action, T&& initial, V0&& v0, V&&... values) noexcept {
        auto next = action(std::forward<T>(initial), std::forward<V0>(v0));
        if constexpr (sizeof...(V) == 0) {
            return next;
        } else {
            return _reduce_with(std::forward<op>(action), std::move(next), std::forward<V>(values)...);
        }
    }
};

//! \addtogroup Utilities
//! \{

namespace concepts {

template<typename A, typename B>
concept same_remove_cvref_t_as = std::is_same_v<std::remove_cvref_t<A>, std::remove_cvref_t<B>>;

}  // namespace concepts

//! \} group Utilities

}  // namespace adac
