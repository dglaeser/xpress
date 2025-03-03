// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup TypeTraits
 * \brief Basic type traits.
 */
#pragma once

#include <type_traits>

#include <cpputils/type_traits.hpp>

#include "utils.hpp"


namespace xp {

//! \addtogroup TypeTraits
//! \{

// bring in all cpputils type traits
using namespace cpputils;

//! Register a type to be a scalar
template<typename T>
struct is_scalar : std::bool_constant<std::is_floating_point_v<T> || std::is_integral_v<T>> {};
template<typename T>
inline constexpr bool is_scalar_v = is_scalar<T>::value;

//! Trait to expose if a type implements `operator[](std::size_t)`
template<typename T>
struct is_indexable : std::bool_constant< requires(const T& t) { {t[std::size_t{}] }; } > {};
template<typename T>
inline constexpr bool is_indexable_v = is_indexable<T>::value;

//! Trait to expose the value_type of e.g. a container
template<typename T>
struct value_type;
template<typename T> requires(is_indexable_v<T>)
struct value_type<T> : std::type_identity<std::remove_cvref_t<decltype(std::declval<T>()[std::size_t{0}])>> {};
template<typename T>
using value_type_t = typename value_type<T>::type;


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct indexable_scalar_type;
    template<typename T> requires(is_indexable_v<T>)
    struct indexable_scalar_type<T> : indexable_scalar_type<value_type_t<T>> {};
    template<typename T> requires(!is_indexable_v<T>)
    struct indexable_scalar_type<T> : std::type_identity<T> {};

}  // namespace detail
#endif  // DOXYGEN

//! Trait to extract the scalar type of a potentially multi-dimensional container
template<typename T>
struct scalar_type;
template<typename T> requires(is_scalar_v<T>)
struct scalar_type<T> : std::type_identity<T> {};
template<typename T> requires(is_indexable_v<T>)
struct scalar_type<T> : detail::indexable_scalar_type<T> {};
template<typename T>
using scalar_type_t = typename scalar_type<T>::type;

//! Trait to get the shape of a multi-dimensional container
template<typename T>
struct shape_of;
template<typename T>
using shape_of_t = typename shape_of<T>::type;

//! Trait (or metafunction) to access values in multi-dimensional containers
template<typename T>
struct access;

//! \} group TypeTraits

//! \addtogroup Concepts
//! \{

//! Two types being the same except for const or reference qualifiers
template<typename A, typename B>
concept same_remove_cvref_t_as = std::is_same_v<std::remove_cvref_t<A>, std::remove_cvref_t<B>>;

//! A type that can be used for tensorial values
template<typename T>
concept tensorial
= std::is_default_constructible_v<std::remove_cvref_t<T>>  // TODO: can we relax this?
and is_complete_v<scalar_type<std::remove_cvref_t<T>>>
and is_complete_v<shape_of<std::remove_cvref_t<T>>>
and is_complete_v<access<std::remove_cvref_t<T>>>
and requires(const T& t) {
    { access<std::remove_cvref_t<T>>::at( *(md_index_iterator{shape_of_t<std::remove_cvref_t<T>>{}}), t ) };
};

//! \} group Concepts

}  // namespace xp
