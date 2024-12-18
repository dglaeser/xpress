// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Solvers
 * \brief Newton solver for finding the roots of nonlinear equations.
 */
#pragma once

#include <optional>
#include <type_traits>
#include <iostream>
#include <string>

#include <xpress/concepts.hpp>
#include <xpress/bindings.hpp>
#include <xpress/expressions.hpp>
#include <xpress/traits.hpp>
#include <xpress/linalg.hpp>

#include "common.hpp"


namespace xp::solvers {

//! \addtogroup Solvers
//! \{

//! Finds the roots of nonlinear equations using Newton's method
template<typename T = double> requires(is_scalar_v<T>)
struct newton {
    constexpr newton(solver_options<T>&& opts) noexcept
    : _opts{std::move(opts)}
    {}

    template<expression E, typename I>
    constexpr auto find_scalar_root_of(const E& equation, bindings<I>&& initial_guess) const noexcept {
        using symbol_t = typename I::symbol_type;
        using value_t = typename I::value_type;
        using result_t = std::optional<value_t>;
        auto root = find_root_of(equation, std::move(initial_guess));
        if (root)
            return result_t{std::move(root).value()[symbol_t{}]};
        return result_t{};
    }

    template<expression E, typename... I>
    constexpr auto find_root_of(const E& equation, bindings<I...>&& initial_guess) const noexcept {
        static_assert(
            !std::conjunction_v<std::is_const<std::remove_reference_t<
                decltype(initial_guess[typename I::symbol_type{}])
            >>...>,
            "Bindings to const refs are not supported as initial guess is updated with the solution."
        );

        using result_t = std::optional<bindings<I...>>;
        using variables = traits::variables_of_t<E>;
        const auto gradient = derivatives_of(equation, variables{});
        auto residual = value_of(equation, initial_guess);

        const auto threshold_squared = _opts.threshold*_opts.threshold;
        std::size_t iteration = 0;
        auto residual_norm_squared = _squared_norm_of(residual);
        while (residual_norm_squared > threshold_squared) {
            if (iteration >= _opts.max_iterations) {
                if (!std::is_constant_evaluated())
                    _logger(1) << " -- Newton solver did not converge after " << iteration << " iterations.\n";
                return result_t{};
            }

            residual = value_of(equation, initial_guess);
            residual_norm_squared = _squared_norm_of(residual);
            _update(initial_guess, residual, gradient.at(initial_guess), variables{});
            ++iteration;
            if (!std::is_constant_evaluated())
                _logger(1) << " -- finished iteration " << iteration << "; residual = " << residual_norm_squared << "\n";
        }

        return result_t{std::move(initial_guess)};
    }

 private:
    constexpr progress_logger _logger(unsigned int verbosity_level) const noexcept {
        return verbosity_level <= _opts.verbosity_level
            ? progress_logger::active(std::cout)
            : progress_logger::suppressed(std::cout);
    }

    template<typename... S, typename R, typename G, typename V>
        requires(is_scalar_v<R>)
    constexpr void _update(bindings<S...>& solution,
                           const R& residual,
                           const G& gradient,
                           const type_list<V>&) const noexcept {
        solution[V{}] -= residual/gradient[V{}];
    }

    template<typename... S, typename R, typename G, typename V1, typename V2>
        requires(tensorial<R>)
    constexpr void _update(bindings<S...>& solution,
                           const R& residual,
                           const G& gradient,
                           const type_list<V1, V2>&) const noexcept {
        static_assert(
            shape_of_t<R>{}.first() == 2,
            "Newton update currently only implemented for scalar equations or 2d equation systems."
        );
        const linalg::tensor jacobian{shape<2, 2>,
            gradient[V1{}][at<0>()], gradient[V2{}][at<0>()],
            gradient[V1{}][at<1>()], gradient[V2{}][at<1>()]
        };
        const auto jacobian_inverse = linalg::tensor{shape<2, 2>,
            jacobian[at<1, 1>()], -jacobian[at<0, 1>()],
            -jacobian[at<1, 0>()], jacobian[at<0, 0>()]
        }*(1.0/linalg::determinant_of(jacobian));
        const auto update = mat_mul(jacobian_inverse, residual);
        solution[V1{}] -= update[at<0>()];
        solution[V2{}] -= update[at<1>()];
    }

    template<typename R> requires(is_scalar_v<R>)
    constexpr auto _squared_norm_of(const R& residual) const noexcept {
        return residual*residual;
    }

    template<typename R> requires(tensorial<R>)
    constexpr auto _squared_norm_of(const R& residual) const noexcept {
        return operators::traits::multiplication_of<R, R>{}(residual, residual);
    }

    solver_options<T> _opts;
};

//! \} group Solvers

}  // namespace xp::solvers
