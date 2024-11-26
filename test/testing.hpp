// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include <boost/ut.hpp>

namespace xp::testing {

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::neq;
using boost::ut::eq;

template<typename A, typename B, typename E = std::common_type_t<A, B>>
inline constexpr bool fuzzy_eq(const A& a, const B& b, const E& eps = 1e-6) noexcept {
    const auto delta = a - b;
    return (delta >= 0 && delta < eps) || (delta < 0 && delta > E{-1}*eps);
}

}  // namespace xp::testing
