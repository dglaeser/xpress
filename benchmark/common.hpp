#pragma once

#include <concepts>
#include <iostream>
#include <vector>
#include <chrono>

namespace adac::benchmark {

template<typename E>
void print_expression_tree_size_to(std::ostream& out, const E&) {
    std::cout << "Expression tree size: " << type_list_size_v<nodes_of_t<E>> << std::endl;
}

class Measurement {
 public:
    void push(double measurement) {
        _measurements.push_back(measurement);
    }

    void write_report_to(std::ostream& out) const {
        double average = 0.0;
        for (const auto& m : _measurements)
            average += m;
        average /= static_cast<double>(_measurements.size());
        out << " -- average runtime: " << average << std::endl;
    }

 private:
    std::vector<double> _measurements;
};

template<std::invocable action>
double measure_invocation(action&& a) {
    auto t1 = std::chrono::steady_clock::now();
    a();
    auto t2 = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(t2 - t1).count();
}

// TODO: instead of size_ts for warmup phase etc, decide automatically based on fluctuations
template<std::invocable action>
Measurement measure(action&& a, std::size_t warmup = 10, std::size_t measurements = 20) {
    std::cout << " -- starting measurement" << std::endl;

    std::cout << " -- warmup phase" << std::endl;
    for (std::size_t i = 0; i < 10; ++i)
        a();

    Measurement measurement;
    std::cout << " -- measurement phase" << std::endl;
    for (std::size_t i = 0; i < 10; ++i)
        measurement.push(measure_invocation(a));
    return measurement;
}

}  // namespace adac::benchmark
