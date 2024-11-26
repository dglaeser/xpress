// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT

#include <iostream>

#ifndef USE_AUTODIFF
#define USE_AUTODIFF 0
#endif

#if USE_AUTODIFF
#include <autodiff/forward/dual.hpp>
#include <autodiff/reverse/var.hpp>
#else
#include <xpress/xp.hpp>
#endif

#include "common.hpp"
#include "benchmark_expression.hpp"

#if USE_AUTODIFF
autodiff::dual f(autodiff::dual _a, autodiff::dual _b) {
    return GENERATE_EXPRESSION(_a, _b);
}
#endif

int main() {
    using namespace xp;

    const double a_value = 2.0;
    const double b_value = 5.0;

#if USE_AUTODIFF
    #if USE_AUTODIFF_BACKWARD
        autodiff::var a = a_value;
        autodiff::var b = b_value;
        auto [measurement, result] = benchmark::measure([&] () {
            const autodiff::var r = GENERATE_EXPRESSION(a, b);
            return r;
        });
    #else
        autodiff::dual a = a_value;
        autodiff::dual b = b_value;
        auto [measurement, result] = benchmark::measure([&] () {
            autodiff::dual r = GENERATE_EXPRESSION(a, b);
            return r;
        });
    #endif
#else
    var a;
    var b;
    auto [measurement, result] = benchmark::measure([&] () {
        return value_of(GENERATE_EXPRESSION(a, b), at(a = a_value, b = b_value));
    });
#endif
    std::cout << "Value = " << result << std::endl;
    measurement.write_report_to(std::cout);

    return 0;
}
