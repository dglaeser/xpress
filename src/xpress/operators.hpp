// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Operators
 * \brief Mathematical operations on symbols/expressions.
 */
#pragma once

#include "traits.hpp"
#include "concepts.hpp"
#include "operators/add.hpp"
#include "operators/subtract.hpp"
#include "operators/multiply.hpp"
#include "operators/divide.hpp"
#include "operators/pow.hpp"
#include "operators/log.hpp"

// operators on tensors
#include "operators/det.hpp"
#include "operators/mat_mul.hpp"


namespace xp {

//! Test two expressions for equality
template<expression A, expression B>
constexpr bool operator==(const A&, const B&) noexcept {
    return traits::is_equal_node_v<A, B>;
}

}  // namespace xp
