#include <array>
#include <algorithm>

#include <xpress/linalg.hpp>

#include "testing.hpp"

int main() {
    using namespace xp::testing;
    using namespace xp;

    "shape_of_trait"_test = [] () {
        static_assert(shape_of_t<std::array<int, 2>>{} == shape<2>);
        static_assert(shape_of_t<std::array<std::array<int, 3>, 2>>{} == shape<2, 3>);
    };

    "tensor_from_array"_test = [] () {
        static constexpr linalg::tensor t{shape<2, 2>, std::array{1, 2, 3, 4}};
        static_assert(t[md_ic<0, 0>] == 1);
        static_assert(t[md_ic<0, 1>] == 2);
        static_assert(t[md_ic<1, 0>] == 3);
        static_assert(t[md_ic<1, 1>] == 4);
    };

    "tensor_from_values"_test = [] () {
        static constexpr linalg::tensor t{shape<2, 2>, 1, 2, 3, 4};
        static_assert(t[md_ic<0, 0>] == 1);
        static_assert(t[md_ic<0, 1>] == 2);
        static_assert(t[md_ic<1, 0>] == 3);
        static_assert(t[md_ic<1, 1>] == 4);
    };

    "tensor_from_scalar"_test = [] () {
        static constexpr linalg::tensor<int, md_shape<2, 2>> t{42};
        static_assert(t[md_ic<0, 0>] == 42);
        static_assert(t[md_ic<0, 1>] == 42);
        static_assert(t[md_ic<1, 0>] == 42);
        static_assert(t[md_ic<1, 1>] == 42);
    };

    "tensor_mat_mul"_test = [] () {
        constexpr linalg::tensor A{shape<2, 3>, 1, 2, 3, 3, 2, 1};
        constexpr linalg::tensor B{shape<3, 2>, 4, 5, 6, 5, 4, 6};
        expect(linalg::mat_mul(A, B) == linalg::tensor{shape<2, 2>, 28, 33, 28, 31});
    };

    "tensor_mat_vec_mul"_test = [] () {
        constexpr linalg::tensor T{shape<2, 2>, 1, 2, 3, 4};
        constexpr linalg::tensor v{shape<2>, 1, 2};
        const auto result = linalg::mat_mul(T, v);
        static_assert(shape_of_t<std::remove_cvref_t<decltype(result)>>{} == shape<2>);
        expect(result == linalg::tensor{shape<2>, 5, 11});
    };

    "tensor_size_t_access"_test = [] () {
        constexpr linalg::tensor A{shape<2, 2>, 1, 2, 3, 4};
        static_assert(A[0, 0] == 1);
        static_assert(A[0, 1] == 2);
        static_assert(A[1, 0] == 3);
        static_assert(A[1, 1] == 4);
        expect(eq(A[0, 0], 1));
        expect(eq(A[0, 1], 2));
        expect(eq(A[1, 0], 3));
        expect(eq(A[1, 1], 4));
    };

    "tensor_size_t_access_row_vector"_test = [] () {
        constexpr linalg::tensor A{shape<1, 3>, 1, 2, 3};
        static_assert(A[0, 0] == 1);
        static_assert(A[0, 1] == 2);
        static_assert(A[0, 2] == 3);
        expect(eq(A[0, 0], 1));
        expect(eq(A[0, 1], 2));
        expect(eq(A[0, 2], 3));
    };

    "tensor_size_t_access_col_vector"_test = [] () {
        constexpr linalg::tensor A{shape<3, 1>, 1, 2, 3};
        static_assert(A[0, 0] == 1);
        static_assert(A[1, 0] == 2);
        static_assert(A[2, 0] == 3);
        expect(eq(A[0, 0], 1));
        expect(eq(A[1, 0], 2));
        expect(eq(A[2, 0], 3));
    };

    "vector_size_t_access_col_vector"_test = [] () {
        constexpr linalg::tensor v{shape<3>, 1, 2, 3};
        static_assert(v[0] == 1);
        static_assert(v[1] == 2);
        static_assert(v[2] == 3);
        expect(eq(v[0], 1));
        expect(eq(v[1], 2));
        expect(eq(v[2], 3));
    };

    "tensor_export"_test = [] () {
        linalg::tensor A{shape<1, 3>, 1, 2, 3};
        std::array<std::array<int, 3>, 1> copied;
        A.export_to(copied);
        expect(std::ranges::equal(copied[0], std::array{1, 2, 3}));
    };

    "tensor_concept"_test = [] () {
        static_assert(tensorial<linalg::tensor<int, md_shape<2, 2>>>);
    };

    return 0;
}
