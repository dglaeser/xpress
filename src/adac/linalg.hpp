// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup LinearAlgebra
 * \brief Algebraic operations on types that represent tensors or vectors.
 */
#pragma once

#include <cstddef>

#include "utils.hpp"


namespace adac::linalg {

//! \addtogroup LinearAlgebra
//! \{

namespace traits {

template<typename T>
struct is_indexable : std::bool_constant< requires(const T& t) { {t[std::size_t{}] }; } > {};
template<typename T>
inline constexpr bool is_indexable_v = is_indexable<T>::value;


#ifndef DOXYGEN
namespace detail {

    template<std::size_t s>
    void _invoke_with_constexpr_size() {}

    template<typename T>
    concept has_constexpr_size = requires(const T& t) {
        { t.size() } -> std::convertible_to<std::size_t>;
        { _invoke_with_constexpr_size<T{}.size()>() };
    };

}  // namespace detail
#endif  // DOXYGEN

template<typename T>
struct size_of;
template<detail::has_constexpr_size T>
struct size_of<T> : std::integral_constant<std::size_t, T{}.size()> {};
template<typename T> requires(is_complete_v<traits::size_of<T>>)
inline constexpr std::size_t size_of_v = size_of<T>::value;

}  // namespace traits


//! \} group LinearAlgebra

}  // namespace adac::linalg
