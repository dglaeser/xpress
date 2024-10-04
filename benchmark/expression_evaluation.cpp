#include <iostream>

#include <adac/adac.hpp>

#include "common.hpp"
#include "test_expression.hpp"

int main() {
    using namespace adac;

    var a;
    var b;
    const auto expression = GENERATE_EXPRESSION(a, b);
    benchmark::print_expression_tree_size_to(std::cout, expression);

    auto measurement = benchmark::measure([&] () {
        evaluate(expression, at(a = 2.0, b = 5.0));
    });
    measurement.write_report_to(std::cout);

    return 0;
}
