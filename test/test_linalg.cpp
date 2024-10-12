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

    "tensor_from_array"_test = [] () {
        static constexpr linalg::tensor t{shape<2, 2>, std::array{1, 2, 3, 4}};
        static_assert(t[md_i_c<0, 0>] == 1);
        static_assert(t[md_i_c<0, 1>] == 2);
        static_assert(t[md_i_c<1, 0>] == 3);
        static_assert(t[md_i_c<1, 1>] == 4);
    };

    "tensor_from_values"_test = [] () {
        static constexpr linalg::tensor t{shape<2, 2>, 1, 2, 3, 4};
        static_assert(t[md_i_c<0, 0>] == 1);
        static_assert(t[md_i_c<0, 1>] == 2);
        static_assert(t[md_i_c<1, 0>] == 3);
        static_assert(t[md_i_c<1, 1>] == 4);
    };

    "tensor_from_scalar"_test = [] () {
        static constexpr linalg::tensor<int, md_shape<2, 2>> t{42};
        static_assert(t[md_i_c<0, 0>] == 42);
        static_assert(t[md_i_c<0, 1>] == 42);
        static_assert(t[md_i_c<1, 0>] == 42);
        static_assert(t[md_i_c<1, 1>] == 42);
    };

    "tensor_mat_mul"_test = [] () {
        constexpr linalg::tensor A{shape<2, 3>, 1, 2, 3, 3, 2, 1};
        constexpr linalg::tensor B{shape<3, 2>, 4, 5, 6, 5, 4, 6};
        expect(linalg::mat_mul(A, B) == linalg::tensor{shape<2, 2>, 28, 33, 28, 31});
    };

    "tensor_concept"_test = [] () {
        static_assert(linalg::concepts::tensor<linalg::tensor<int, md_shape<2, 2>>>);
    };

    return 0;
}
