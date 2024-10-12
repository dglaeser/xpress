// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Solvers
 * \brief Common options and helpers used in the solvers.
 */
#pragma once

#include <utility>
#include <type_traits>

#include <adac/bindings.hpp>


namespace adac::solvers {

//! \addtogroup Solvers
//! \{

//! Convenience function to create bindings for an initial guess
template<concepts::binder... B>
inline constexpr auto starting_from(B&&... b) noexcept {
    return bindings{std::forward<B>(b)...};
}

//! Basic options for iterative solvers
template<typename T = double>
struct solver_options {
    T threshold;
    std::size_t max_iterations;
};

//! \} group Solvers

}  // namespace adac::solvers
