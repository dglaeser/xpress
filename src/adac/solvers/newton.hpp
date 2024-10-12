// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Solvers
 * \brief Newton solver for finding the roots of nonlinear equations.
 */
#pragma once

#include <type_traits>

#include <adac/concepts.hpp>
#include <adac/bindings.hpp>
#include <adac/expressions.hpp>
#include <adac/traits.hpp>

#include "common.hpp"


namespace adac::solvers {

//! \addtogroup Solvers
//! \{

//! Finds the roots of nonlinear equations using Newton's method
template<typename T = double> requires(traits::is_scalar_v<T>)
struct newton {
    constexpr newton(solver_options<T>&& opts) noexcept
    : _opts{std::move(opts)}
    {}

    template<concepts::expression E, typename I>
    constexpr auto find_root_of(const E& equation, bindings<I>&& initial_guess) const noexcept {
        static_assert(
            !std::conjunction_v<std::is_const<std::remove_reference_t<decltype(initial_guess[typename I::symbol_type{}])>>>,
            "Bindings to const refs are not supported as initial guess is updated with the solution."
        );

        using variables = traits::variables_of_t<E>;
        const auto gradient = derivatives_of(equation, variables{});
        auto residual = value_of(equation, initial_guess);

        std::size_t iteration = 0;
        const auto threshold_squared = _opts.threshold*_opts.threshold;
        while (residual*residual > threshold_squared && iteration < _opts.max_iterations) {
            residual = value_of(equation, initial_guess);
            _update(initial_guess, residual, gradient.at(initial_guess), variables{});
            iteration++;
        }

        return initial_guess;
    }

 private:
    template<typename... S, typename R, typename G, typename V>
    constexpr void _update(bindings<S...>& solution,
                           const R& residual,
                           const G& gradient,
                           const type_list<V>&) const noexcept {
        solution[V{}] -= residual/gradient[V{}];
    }

    solver_options<T> _opts;
};

//! \} group Solvers

}  // namespace adac::solvers
