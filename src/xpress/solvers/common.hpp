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
#include <ostream>

#include <xpress/bindings.hpp>


namespace xp::solvers {

//! \addtogroup Solvers
//! \{

//! Convenience function to create bindings for an initial guess
template<binder... B>
inline constexpr auto starting_from(B&&... b) noexcept {
    return bindings{std::forward<B>(b)...};
}

//! Basic options for iterative solvers
template<typename T = double>
struct solver_options {
    T threshold;
    std::size_t max_iterations;
    unsigned int verbosity_level = 0;
};

//! Small wrapper around an std::ostream to activate/deactivate progress output
class progress_logger {
 public:
    static constexpr progress_logger suppressed(std::ostream& s) noexcept { return {s, true}; }
    static constexpr progress_logger active(std::ostream& s) noexcept { return {s, false}; }

    template<typename T>
    constexpr progress_logger& operator<<(T&& t) noexcept {
        if (_output_active)
            _s << std::forward<T>(t);
        return *this;
    }

 private:
    constexpr progress_logger(std::ostream& s, bool suppress_output) noexcept
    : _s{s}
    , _output_active{!suppress_output}
    {}

    std::ostream& _s;
    bool _output_active;
};

//! \} group Solvers

}  // namespace xp::solvers
