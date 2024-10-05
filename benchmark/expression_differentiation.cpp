#include <iostream>

#include <adac/adac.hpp>

#include "common.hpp"
#include "benchmark_expression.hpp"

int main() {
    using namespace adac;

    var a;
    var b;
    const auto derivative = differentiate(GENERATE_EXPRESSION(a, b), wrt(a));
    benchmark::print_expression_tree_size_to(std::cout, derivative);

    auto measurement = benchmark::measure([&] () {
        evaluate(derivative, at(a = 2.0, b = 5.0));
    });
    measurement.write_report_to(std::cout);

    return 0;
}
