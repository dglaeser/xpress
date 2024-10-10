#include <adac/linalg.hpp>

#include "testing.hpp"

int main() {
    using namespace adac::testing;
    using namespace adac;

    "size_of_trait"_test = [] () {
        static_assert(linalg::traits::size_of_v<std::array<int, 2>> == 2);
        static_assert(linalg::traits::size_of_v<std::array<std::array<int, 3>, 2>> == 2);
    };

    "shape_of_trait"_test = [] () {
        static_assert(linalg::traits::shape_of_t<std::array<int, 2>>{} == shape<2>);
        static_assert(linalg::traits::shape_of_t<std::array<std::array<int, 3>, 2>>{} == shape<2, 3>);
    };

    return 0;
}
