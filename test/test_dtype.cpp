// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT

#include <string>

#include <xpress/dtype.hpp>
#include <xpress/concepts.hpp>
#include <xpress/linalg.hpp>

#include "testing.hpp"

template<typename T1, typename T2>
inline constexpr bool verify_same = std::is_same_v<T1, T2>;

int main() {
    using namespace xp::testing;
    using namespace xp::dtype;
    using namespace xp::traits;
    using namespace xp;

    static_assert(bindable_to<const double&, real>);
    static_assert(bindable_to<double&, real>);
    static_assert(bindable_to<int, real>);
    static_assert(bindable_to<linalg::tensor<double, md_shape<2, 2>>, real>);

    static_assert(!bindable_to<std::string, real>);
    static_assert(!bindable_to<linalg::tensor<double, md_shape<2, 2>>, integral>);
    static_assert(bindable_to<linalg::tensor<int, md_shape<2, 2>>, integral>);

    static_assert(verify_same<common_dtype_t<integral, real>, real>);
    static_assert(verify_same<common_dtype_t<real, integral>, real>);

    static_assert(verify_same<common_dtype_t<integral, double>, real>);
    static_assert(verify_same<common_dtype_t<double, integral>, real>);

    static_assert(verify_same<common_dtype_t<real, double>, real>);
    static_assert(verify_same<common_dtype_t<double, real>, real>);

    static_assert(verify_same<common_dtype_t<any, double>, any>);
    static_assert(verify_same<common_dtype_t<double, any>, any>);

    static_assert(verify_same<common_dtype_t<integral, integral>, integral>);
    static_assert(verify_same<common_dtype_t<real, real>, real>);

    static_assert(verify_same<common_dtype_t<integral, integral, integral, any>, any>);
    static_assert(verify_same<common_dtype_t<integral, integral, integral, real>, real>);

    return 0;
}
