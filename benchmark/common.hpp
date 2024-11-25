// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>
#include <concepts>
#include <iostream>
#include <utility>
#include <vector>
#include <chrono>

namespace xp::benchmark {

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
        out << "average runtime: " << average << std::endl;
    }

 private:
    std::vector<double> _measurements;
};

template<std::invocable action>
auto measure_invocation(action&& a) {
    auto t1 = std::chrono::steady_clock::now();
    auto result = a();
    auto t2 = std::chrono::steady_clock::now();
    return std::make_pair(std::chrono::duration<double>(t2 - t1).count(), result);
}

// TODO: instead of size_ts for warmup phase etc, decide automatically based on fluctuations
template<std::invocable action>
auto measure(action&& a, std::size_t warmup = 10, std::size_t measurements = 20) {
    for (std::size_t i = 0; i < warmup; ++i)
        a();

    Measurement measurement;
    for (std::size_t i = 0; i < measurements - 1; ++i) {
        auto [runtime, _] = measure_invocation(a);
        measurement.push(runtime);
    }

    auto [runtime, result] = measure_invocation(a);
    measurement.push(runtime);

    return std::make_pair(measurement, result);
}

}  // namespace xp::benchmark
