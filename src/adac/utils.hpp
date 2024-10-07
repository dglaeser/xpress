// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Utilities
 * \brief Utility classes.
 */

#pragma once

#include <type_traits>

#include <cpputils/type_traits.hpp>
#include <cpputils/utility.hpp>


namespace adac {

//! bring in all cpputils utility
using namespace cpputils;

//! \addtogroup Utilities
//! \{

namespace concepts {

template<typename A, typename B>
concept same_remove_cvref_t_as = std::is_same_v<std::remove_cvref_t<A>, std::remove_cvref_t<B>>;

}  // namespace concepts

//! \} group Utilities

}  // namespace adac
