// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Utilities
 * \brief Utility classes.
 */

#pragma once

#include <ostream>
#include <utility>
#include <type_traits>

#include <cpputils/type_traits.hpp>
#include <cpputils/utility.hpp>


namespace adac {

//! \addtogroup Utilities
//! \{

//! bring in all cpputils utilities
using namespace cpputils;

//! Instance of a compile-time index
template<std::size_t i>
inline constexpr index_constant<i> i_c{};


namespace traits {

template<auto a, auto b> struct is_equal : std::bool_constant<(a == b)> {};

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

    //! Return the first value in the list
    static constexpr auto first() noexcept {
        return at(index_constant<0>{});
    }

    //! Return the last value in the list
    static constexpr auto last() noexcept {
        return at(index_constant<size-1>{});
    }

    //! Return the value at the given index in the list
    template<std::size_t i> requires(i < size)
    static constexpr auto at(index_constant<i> idx) noexcept {
        using base = detail::values<std::make_index_sequence<size>, v...>;
        return base::at(idx);
    }

    //! Perform a reduction operation on this list
    template<typename op, typename T>
    static constexpr auto reduce_with(op&& action, T&& initial) noexcept {
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
        (s << ... << (std::to_string(v) + ","));
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

//! Type to represent a multi-dimensional shape
template<std::size_t... s>
struct md_shape {
    static constexpr std::size_t size = sizeof...(s);
    static constexpr std::size_t count = value_list<s...>{}.reduce_with(std::multiplies{}, std::size_t{1});

    template<std::size_t... v> requires(size > 0)
    constexpr bool operator==(const md_shape<v...>&) const noexcept { return false; }
    constexpr bool operator==(const md_shape&) const noexcept { return true; }

    template<std::size_t _i> requires(_i < size)
    static constexpr std::size_t at(const index_constant<_i>& idx) noexcept {
        return value_list<s...>::at(idx);
    }

    friend std::ostream& operator<<(std::ostream& out, const md_shape&) {
        out << "[";
        (out << ... << (std::to_string(s) + ","));
        out << "]";
        return out;
    }
};

template<std::size_t... s>
inline constexpr md_shape<s...> shape{};

//! Type to represent a multi-dimensional index
template<std::size_t... i>
struct md_index {
    static constexpr std::size_t size = sizeof...(i);

    template<std::size_t... v> requires(size > 0)
    constexpr bool operator==(const md_index<v...>&) const noexcept { return false; }
    constexpr bool operator==(const md_index&) const noexcept { return true; }

    template<std::size_t _i> requires(_i < size)
    static constexpr auto at(const index_constant<_i>&) noexcept {
        return index_constant<value_list<i...>::at(index_constant<_i>{})>{};
    }

    template<std::size_t s0, std::size_t... s> requires(sizeof...(s) == size - 1)
    static constexpr auto as_flat_index_in(const md_shape<s0, s...>&) noexcept {
        return i_c<_sum_up_flat_index<0>(md_shape<s...>{}, 0)>;
    }

    static constexpr auto as_flat_index_in(const md_shape<>&) noexcept requires(size == 0) {
        return i_c<0>;
    }

 private:
    template<std::size_t _i, std::size_t s0, std::size_t... s>
    static constexpr std::size_t _sum_up_flat_index(const md_shape<s0, s...>& sub_shape, std::size_t current) noexcept {
        return _sum_up_flat_index<_i + 1>(
            md_shape<s...>{},
            current + at(i_c<_i>).value*value_list<s0, s...>{}.reduce_with(std::multiplies{}, std::size_t{1})
        );
    }

    template<std::size_t _i>
    static constexpr std::size_t _sum_up_flat_index(const md_shape<>&, std::size_t current) noexcept {
        return current + at(i_c<_i>).value;
    }
};

template<std::size_t... i>
inline constexpr md_index<i...> md_i_c{};


#ifndef DOXYGEN
namespace detail {

    template<typename shape>
    struct first_index_in;
    template<std::size_t... s>
    struct first_index_in<md_shape<s...>> : std::type_identity<md_index<(s*0)...>> {};

}  // namespace detail
#endif  // DOXYGEN

//! Iterator over multi-dimensional indices
template<typename shape, typename current = typename detail::first_index_in<shape>::type>
struct md_index_iterator;

template<std::size_t... s, std::size_t... i> requires(sizeof...(s) == sizeof...(i))
struct md_index_iterator<md_shape<s...>, md_index<i...>> {
    constexpr md_index_iterator(const md_shape<s...>&) noexcept {}
    constexpr md_index_iterator(const md_shape<s...>&, const md_index<i...>&) noexcept {}

    static constexpr bool is_incrementable() noexcept {
        return md_index<i...>::as_flat_index_in(md_shape<s...>{}).value < md_shape<s...>::count - 1;
    }

    static constexpr auto incremented() noexcept {
        static_assert(
            md_index<i...>::as_flat_index_in(md_shape<s...>{}).value < md_shape<s...>::count - 1,
            "End of the range has already been reached"
        );
        return adac::md_index_iterator{md_shape<s...>{}, _incremented<sizeof...(s) - 1, false>(md_index<>{})};
    }

    constexpr md_index<i...> operator*() const noexcept {
        return {};
    }

 private:
    template<std::size_t _i, bool was_incremented, std::size_t... c>
    static constexpr auto _incremented(md_index<c...> intermediate_result) noexcept {
        constexpr std::size_t at_i = md_index<i...>::at(i_c<_i>).value;
        constexpr bool do_increment = !was_incremented && at_i < md_shape<s...>::at(i_c<_i>) - 1;
        constexpr bool do_zero = !was_incremented && at_i >= md_shape<s...>::at(i_c<_i>) - 1;
        constexpr auto update = [&] () constexpr {
            if constexpr (do_increment)
                return md_index<at_i+1, c...>{};
            else if constexpr (do_zero)
                return md_index<0, c...>{};
            else
                return md_index<at_i, c...>{};
        } ();

        if constexpr (_i > 0)
            return _incremented<_i - 1, was_incremented || do_increment>(update);
        else
            return update;
    }
};

template<typename shape>
md_index_iterator(const shape&) -> md_index_iterator<shape, typename detail::first_index_in<shape>::type>;

template<typename shape, typename index>
md_index_iterator(const shape&, const index&) -> md_index_iterator<shape, index>;

//! Visit all multi-dimensional indices in the given shape
template<typename visitor, std::size_t... s>
    requires(std::invocable<visitor, md_index<(s*0)...>>)
inline constexpr void visit_indices_in(const md_shape<s...>& shape, visitor&& v) noexcept {
    const auto index_visit = [&] <typename... T> (this auto self, const md_index_iterator<T...>& it) constexpr {
        v(*it);
        if constexpr (md_index_iterator<T...>::is_incrementable())
            self(it.incremented());
    };
    index_visit(md_index_iterator{shape});
}

namespace concepts {

template<typename A, typename B>
concept same_remove_cvref_t_as = std::is_same_v<std::remove_cvref_t<A>, std::remove_cvref_t<B>>;

}  // namespace concepts

//! \} group Utilities

}  // namespace adac
