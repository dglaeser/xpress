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


namespace xp {

//! \addtogroup Utilities
//! \{

//! bring in all cpputils utilities
using namespace cpputils;

//! Null type
struct none {};

//! Type to represent a multi-dimensional shape
template<std::size_t... s>
struct md_shape {
    using as_values_t = values<s...>;

    static constexpr std::size_t size = sizeof...(s);
    static constexpr std::size_t count = values<s...>{}.reduce_with(std::multiplies{}, std::size_t{1});
    static constexpr bool is_square = sizeof...(s) == 2 && std::conjunction_v<is_equal<s, values<s...>::first()>...>;

    constexpr md_shape() = default;
    constexpr md_shape(const values<s...>) noexcept {}

    template<std::size_t... v> requires(size > 0)
    constexpr bool operator==(const md_shape<v...>&) const noexcept { return false; }
    constexpr bool operator==(const md_shape&) const noexcept { return true; }

    //! Return the first dimension of this shape
    static constexpr auto first() noexcept {
        return at(index_constant<0>{});
    }

    //! Return the last dimension of this shape
    static constexpr auto last() noexcept {
        return at(index_constant<size-1>{});
    }

    template<std::size_t _i> requires(_i < size)
    static constexpr std::size_t at(const index_constant<_i>& idx) noexcept {
        return values<s...>::at(idx);
    }

    friend std::ostream& operator<<(std::ostream& out, const md_shape&) {
        out << "<";
        out << values<s...>{};
        out << ">";
        return out;
    }
};

//! Instance of md_shape
template<std::size_t... s>
inline constexpr md_shape<s...> shape{};

//! Type to represent a multi-dimensional index
template<std::size_t... i>
struct md_index {
    static constexpr std::size_t size = sizeof...(i);

    constexpr md_index() = default;
    constexpr md_index(const values<i...>) noexcept {}

    template<std::size_t... v> requires(size > 0)
    constexpr bool operator==(const md_index<v...>&) const noexcept { return false; }
    constexpr bool operator==(const md_index&) const noexcept { return true; }

    template<std::size_t _i> requires(_i < size)
    static constexpr auto at(const index_constant<_i>&) noexcept {
        return index_constant<values<i...>::at(index_constant<_i>{})>{};
    }

    template<std::size_t _i>
    static constexpr auto with_prepended(const index_constant<_i>&) noexcept {
        return md_index<_i, i...>{};
    }

    template<std::size_t _i>
    static constexpr auto with_appended(const index_constant<_i>&) noexcept {
        return md_index<i..., _i>{};
    }

    template<std::size_t s0, std::size_t... s> requires(sizeof...(s) == size - 1)
    static constexpr auto as_flat_index_in(const md_shape<s0, s...>&) noexcept {
        return ic<_sum_up_flat_index<0>(md_shape<s...>{}, 0)>;
    }

    static constexpr auto as_flat_index_in(const md_shape<>&) noexcept requires(size == 0) {
        return ic<0>;
    }

    template<std::size_t... s>
    static constexpr bool is_contained_in(const md_shape<s...>&) noexcept {
        if constexpr (sizeof...(s) != sizeof...(i))
            return false;
        else
            return std::conjunction_v<is_less<i, s>...>;
    }

 private:
    template<std::size_t _i, std::size_t s0, std::size_t... s>
    static constexpr std::size_t _sum_up_flat_index(const md_shape<s0, s...>&, std::size_t current) noexcept {
        return _sum_up_flat_index<_i + 1>(
            md_shape<s...>{},
            current + at(ic<_i>).value*values<s0, s...>{}.reduce_with(std::multiplies{}, std::size_t{1})
        );
    }

    template<std::size_t _i>
    static constexpr std::size_t _sum_up_flat_index(const md_shape<>&, std::size_t current) noexcept {
        return current + at(ic<_i>).value;
    }
};

//! Instance of a md_index
template<std::size_t... i>
inline constexpr md_index<i...> md_ic{};

//! Factory function to create an md_index from the given indices
template<std::size_t... i> requires(sizeof...(i) > 0)
inline constexpr auto at() noexcept {
    return md_index<i...>{};
}


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
        return xp::md_index_iterator{md_shape<s...>{}, _incremented<sizeof...(s) - 1, false>(md_index<>{})};
    }

    constexpr md_index<i...> operator*() const noexcept {
        return {};
    }

 private:
    template<std::size_t _i, bool was_incremented, std::size_t... c>
    static constexpr auto _incremented(md_index<c...>) noexcept {
        constexpr std::size_t at_i = md_index<i...>::at(ic<_i>).value;
        constexpr bool do_increment = !was_incremented && at_i < md_shape<s...>::at(ic<_i>) - 1;
        constexpr bool do_zero = !was_incremented && at_i >= md_shape<s...>::at(ic<_i>) - 1;
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

}  // namespace xp
