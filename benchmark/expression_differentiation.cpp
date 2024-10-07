#include <iostream>

#ifndef USE_AUTODIFF
#define USE_AUTODIFF 0
#endif

#if USE_AUTODIFF
#include <autodiff/forward/dual.hpp>
#include <autodiff/reverse/var.hpp>
#else
#include <adac/adac.hpp>
#endif

#include "common.hpp"
#include "benchmark_expression.hpp"

using namespace adac;

#if USE_AUTODIFF
autodiff::dual f(autodiff::dual _a, autodiff::dual _b) {
    return GENERATE_EXPRESSION(_a, _b);
}
#endif

int main() {

    const double a_value = 2.0;
    const double b_value = 5.0;

#if USE_AUTODIFF
    #if USE_AUTODIFF_BACKWARD
        autodiff::var a = a_value;
        autodiff::var b = b_value;
        auto [measurement, result] = benchmark::measure([&] () {
            const autodiff::var expression = GENERATE_EXPRESSION(a, b);
            const auto [d_da, d_db] = derivatives(expression, wrt(a, b));
            return std::make_tuple(d_da, d_db);
        });
    #else
        autodiff::dual a = a_value;
        autodiff::dual b = b_value;
        auto [measurement, result] = benchmark::measure([&] () {
            auto d_da = derivative(f, wrt(a), at(a, b));
            auto d_db = derivative(f, wrt(b), at(a, b));
            return std::make_tuple(d_da, d_db);
        });
    #endif
#else
    var a;
    var b;
    auto [measurement, result] = benchmark::measure([&] () {
        const auto derivs = differentiate(GENERATE_EXPRESSION(a, b), wrt(a, b));
        const auto d_da = evaluate(derivs.wrt(a), at(a = a_value, b = b_value));
        const auto d_db = evaluate(derivs.wrt(b), at(a = a_value, b = b_value));
        return std::make_tuple(d_da, d_db);
    });
#endif

    std::cout << "d_da = " << std::get<0>(result) << std::endl;
    std::cout << "d_db = " << std::get<1>(result) << std::endl;
    measurement.write_report_to(std::cout);

    return 0;
}
