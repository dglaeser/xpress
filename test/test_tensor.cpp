#include <array>
#include <utility>
#include <algorithm>
#include <type_traits>
#include <sstream>

#include <adac/operators.hpp>
#include <adac/symbols.hpp>
#include <adac/tensor.hpp>

#include "testing.hpp"


template<typename T, std::size_t r, std::size_t c>
class matrix {
    constexpr matrix(std::array<std::array<T, c>, r>&& data)
    : _data{std::move(data)}
    {}

 public:
    static constexpr std::size_t rows = r;
    static constexpr std::size_t cols = c;
    static constexpr std::size_t size = rows;

    constexpr matrix() = default;

    template<typename self>
    constexpr decltype(auto) operator[](this self&& s, std::size_t i) noexcept {
        return s._data[i];
    }

    constexpr matrix operator*(T factor) const noexcept {
        auto new_data = _data;
        std::ranges::for_each(new_data, [&] (auto& row) {
            std::ranges::for_each(row, [&] (auto& value) {
                value *= factor;
            });
        });
        return matrix{std::move(new_data)};
    }

    constexpr bool operator==(const matrix& other) const noexcept {
        for (std::size_t i = 0; i < r; ++i)
            for (std::size_t j = 0; j < c; ++j)
                if (_data[i][j] != other._data[i][j])
                    return false;
        return true;
    }

 private:
    std::array<std::array<T, c>, r> _data;
};


int main() {
    using namespace adac::testing;
    using namespace adac;

    "tensor_uniqueness_test"_test = [] () {
        tensor t1{shape<2, 3>};
        tensor t2{shape<2, 3>};
        static_assert(!std::is_same_v<decltype(t1), decltype(t2)>);
    };

    "tensor_value"_test = [] () {
        matrix<int, 2, 2> m{};
        tensor t{shape<2, 2>};
        static_assert(std::is_same_v<decltype(value_of(t, at(t = m))), const matrix<int, 2, 2>&>);
    };

    "tensor_addition"_test = [] () {
        tensor t1{shape<2, 2>};
        tensor t2{shape<2, 2>};
        expect(value_of(t1 + t2, at(
            t1 = linalg::tensor{shape<2, 2>, 1, 2, 3, 4},
            t2 = linalg::tensor{shape<2, 2>, 2, 3, 4, 5}
        )) == linalg::tensor{shape<2, 2>, 3, 5, 7, 9});
    };

    "tensor_subtraction"_test = [] () {
        tensor t1{shape<2, 2>};
        tensor t2{shape<2, 2>};
        expect(value_of(t1 - t2, at(
            t1 = linalg::tensor{shape<2, 2>, 1, 2, 3, 4},
            t2 = linalg::tensor{shape<2, 2>, 2, 3, 4, 5}
        )) == linalg::tensor{shape<2, 2>, -1, -1, -1, -1});
    };

    "tensor_times_scalar"_test = [] () {
        matrix<int, 2, 2> m;
        m[0][0] = 1; m[0][1] = 2;
        m[1][0] = 3; m[1][1] = 4;

        tensor t{shape<2, 2>};
        expect(value_of(t*val<2>, at(t = m)) == m*2);
        expect(value_of(val<2>*t, at(t = m)) == m*2);
    };

    "tensor_divided_by_scalar"_test = [] () {
        matrix<int, 2, 2> m;
        m[0][0] = 2; m[0][1] = 4;
        m[1][0] = 6; m[1][1] = 8;

        matrix<int, 2, 2> expected;
        expected[0][0] = 1; expected[0][1] = 2;
        expected[1][0] = 3; expected[1][1] = 4;

        tensor t{shape<2, 2>};
        expect(value_of(t/val<2>, at(t = m)) == expected);
    };

    "tensor_stream"_test = [] () {
        tensor T{shape<2, 3>};
        std::ostringstream s;
        write_to(s, T, at(T = "T<2, 3>"));
        expect(eq(s.str(), std::string{"T<2, 3>"}));
    };

    "tensor_scalar_product"_test = [] () {
        matrix<int, 2, 2> m;
        m[0][0] = 1; m[0][1] = 2;
        m[1][0] = 3; m[1][1] = 4;

        const tensor t1{shape<2, 2>};
        const tensor t2{shape<2, 2>};
        expect(eq(value_of(t1*t2, at(t1 = m, t2 = m)), 1+4+9+16));
    };

    "tensor_is_square"_test = [] () {
        static_assert(tensor{shape<2, 2>}.is_square);
        static_assert(tensor{shape<3, 3>}.is_square);
        static_assert(!tensor{shape<2, 2, 2>}.is_square);
        static_assert(!tensor{shape<2, 3>}.is_square);
    };

    "tensor_determinant"_test = [] () {
        const tensor T22{shape<2, 2>};
        const tensor T33{shape<3, 3>};
        expect(eq(value_of(det(T22), at(T22 = linalg::tensor{shape<2, 2>, 3, 8, 4, 6})), -14));
        expect(eq(value_of(det(T33), at(T33 = linalg::tensor{shape<3, 3>, 6, 1, 1, 4, -2, 5, 2, 8, 7})), -306));
    };

    "tensor_bound_determinant"_test = [] () {
        const tensor T{shape<2, 2>};
        const auto detT = value_of(det(T), at(T = linalg::tensor{shape<2, 2>, 3, 8, 4, 6}));
        expect(eq(detT, -14));
        expect(eq(value_of(det(T), at(det(T) = detT)), -14));
    };

    "tensor_variable_access"_test = [] () {
        constexpr tensor t{shape<2, 2>};
        constexpr auto T00 = t[at<0, 0>()];
        constexpr auto T01 = t[at<0, 1>()];
        constexpr auto T10 = t[at<1, 0>()];
        constexpr auto T11 = t[at<1, 1>()];

        using T = std::remove_cvref_t<decltype(t)>;
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(T00)>, tensor_var<T, 0, 0>>);
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(T01)>, tensor_var<T, 0, 1>>);
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(T10)>, tensor_var<T, 1, 0>>);
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(T11)>, tensor_var<T, 1, 1>>);
    };

    "tensor_variable_value"_test = [] () {
        constexpr tensor t{shape<2, 2>};
        constexpr linalg::tensor t_value{shape<2, 2>, 1, 2, 3, 4};
        static_assert(value_of(t[at<0, 0>()], at(t = t_value)) == 1);
        static_assert(value_of(t[at<0, 1>()], at(t = t_value)) == 2);
        static_assert(value_of(t[at<1, 0>()], at(t = t_value)) == 3);
        static_assert(value_of(t[at<1, 1>()], at(t = t_value)) == 4);
    };

    "tensor_variable_stream"_test = [] () {
        constexpr tensor t{shape<2, 2>};
        constexpr auto T00 = t[at<0, 0>()];
        constexpr auto T01 = t[at<0, 1>()];
        constexpr auto T10 = t[at<1, 0>()];
        constexpr auto T11 = t[at<1, 1>()];

        { std::ostringstream s; write_to(s, T00, at(t = "M")); expect(eq(s.str(), std::string{"M[0, 0]"})); }
        { std::ostringstream s; write_to(s, T01, at(t = "M")); expect(eq(s.str(), std::string{"M[0, 1]"})); }
        { std::ostringstream s; write_to(s, T10, at(t = "M")); expect(eq(s.str(), std::string{"M[1, 0]"})); }
        { std::ostringstream s; write_to(s, T11, at(t = "M")); expect(eq(s.str(), std::string{"M[1, 1]"})); }
    };

    "tensor_2x2_determinant_derivative"_test = [] () {
        const linalg::tensor value{shape<2, 2>, 1.0, 2.0, 3.0, 4.0};
        const auto determinant = -2.0;
        const linalg::tensor inverse = linalg::tensor{shape<2, 2>, 4.0, -2.0, -3.0, 1.0}*(1.0/determinant);
        const linalg::tensor inverse_transposed = linalg::tensor{shape<2, 2>,
            inverse[at<0, 0>()], inverse[at<1, 0>()],
            inverse[at<0, 1>()], inverse[at<1, 1>()]
        };
        // see https://en.wikipedia.org/wiki/Tensor_derivative_(continuum_mechanics)#Derivative_of_the_determinant_of_a_second-order_tensor
        const linalg::tensor expected = inverse_transposed*determinant;

        const tensor T{shape<2, 2>};
        const auto ddetT_dT = derivative_of(det(T), wrt(T), at(T = value));
        expect(fuzzy_eq(ddetT_dT[at<0, 0>()], expected[at<0, 0>()]));
        expect(fuzzy_eq(ddetT_dT[at<0, 1>()], expected[at<0, 1>()]));
        expect(fuzzy_eq(ddetT_dT[at<1, 0>()], expected[at<1, 0>()]));
        expect(fuzzy_eq(ddetT_dT[at<1, 1>()], expected[at<1, 1>()]));
    };

    "tensor_3x3_determinant_derivative"_test = [] () {
        const linalg::tensor value{shape<3, 3>,
            1.0, 2.0, 3.0,
            3.0, 2.0, 1.0,
            2.0, 1.0, 3.0
        };
        // computed with wolfram alpha
        const auto determinant = -12.0;
        const linalg::tensor inverse_transposed = linalg::tensor{shape<3, 3>,
            -5.0, 7.0, 1.0,
            3.0, 3.0, -3.0,
            4.0, -8.0, 4.0
        }*(1.0/12.0);
        // see https://en.wikipedia.org/wiki/Tensor_derivative_(continuum_mechanics)#Derivative_of_the_determinant_of_a_second-order_tensor
        const linalg::tensor expected = inverse_transposed*determinant;

        const tensor T{shape<3, 3>};
        const auto ddetT_dT = derivative_of(det(T), wrt(T), at(T = value));
        expect(fuzzy_eq(ddetT_dT[at<0, 0>()], expected[at<0, 0>()]));
        expect(fuzzy_eq(ddetT_dT[at<0, 1>()], expected[at<0, 1>()]));
        expect(fuzzy_eq(ddetT_dT[at<0, 2>()], expected[at<0, 2>()]));
        expect(fuzzy_eq(ddetT_dT[at<1, 0>()], expected[at<1, 0>()]));
        expect(fuzzy_eq(ddetT_dT[at<1, 1>()], expected[at<1, 1>()]));
        expect(fuzzy_eq(ddetT_dT[at<1, 2>()], expected[at<1, 2>()]));
        expect(fuzzy_eq(ddetT_dT[at<2, 0>()], expected[at<2, 0>()]));
        expect(fuzzy_eq(ddetT_dT[at<2, 1>()], expected[at<2, 1>()]));
        expect(fuzzy_eq(ddetT_dT[at<2, 2>()], expected[at<2, 2>()]));
    };

    "vector_scalar_product"_test = [] () {
        constexpr std::array<int, 2> data{1, 2};
        static constexpr vector<2> v1{};
        static constexpr vector<2> v2{};
        expect(eq(value_of(v1*v2, at(v1 = data, v2 = data)), 5));
    };

    "tensor_scalar_product_derivative"_test = [] () {
        matrix<int, 2, 2> m1;
        matrix<int, 2, 2> m2;
        m1[0][0] = 1; m1[0][1] = 2; m1[1][0] = 3; m1[1][1] = 4;
        m2[0][0] = 10; m2[0][1] = 11; m2[1][0] = 12; m2[1][1] = 13;
        static constexpr tensor t1{shape<2, 2>};
        static constexpr tensor t2{shape<2, 2>};
        static constexpr auto expr = val<42>*(t1*t2);
        const auto d_dv1 = value_of(derivative_of(expr, wrt(t1)), at(t1 = m1, t2 = m2));
        const auto d_dv2 = value_of(derivative_of(expr, wrt(t2)), at(t1 = m1, t2 = m2));
        expect(eq(d_dv1[0][0], m2[0][0]*42)); expect(eq(d_dv1[0][1], m2[0][1]*42));
        expect(eq(d_dv1[1][0], m2[1][0]*42)); expect(eq(d_dv1[1][1], m2[1][1]*42));

        expect(eq(d_dv2[0][0], m1[0][0]*42)); expect(eq(d_dv2[0][1], m1[0][1]*42));
        expect(eq(d_dv2[1][0], m1[1][0]*42)); expect(eq(d_dv2[1][1], m1[1][1]*42));
    };

    "vector_scalar_product_derivative"_test = [] () {
        constexpr std::array<int, 2> data1{1, 2};
        constexpr std::array<int, 2> data2{42, 43};
        static constexpr vector<2> v1{};
        static constexpr vector<2> v2{};
        static constexpr auto expr = val<42>*(v1*v2);
        const auto d_dv1 = value_of(derivative_of(expr, wrt(v1)), at(v1 = data1, v2 = data2));
        const auto d_dv2 = value_of(derivative_of(expr, wrt(v2)), at(v1 = data1, v2 = data2));
        expect(eq(d_dv1[0], data2[0]*42));
        expect(eq(d_dv1[1], data2[1]*42));
        expect(eq(d_dv2[0], data1[0]*42));
        expect(eq(d_dv2[1], data1[1]*42));
    };

    "vector_expression_value"_test = [] () {
        var a;
        var b;
        auto v = vector_expression::from(a, b);
        static_assert(std::is_same_v<decltype(v[i_c<0>]), decltype(a)>);
        static_assert(std::is_same_v<decltype(v[i_c<1>]), decltype(b)>);
        static_assert(value_of(v, at(a = 42, b = 43))[i_c<0>] == 42);
        static_assert(value_of(v, at(a = 42, b = 43))[i_c<1>] == 43);
        expect(eq(value_of(v, at(a = 42, b = 43))[i_c<0>], 42));
        expect(eq(value_of(v, at(a = 42, b = 43))[i_c<1>], 43));
    };

    "tensor_expression_value"_test = [] () {
        var a;
        var b;
        tensor_expression v{shape<2>, a, b};
        static_assert(std::is_same_v<decltype(v[md_i_c<0>]), decltype(a)>);
        static_assert(std::is_same_v<decltype(v[md_i_c<1>]), decltype(b)>);
        static_assert(value_of(v, at(a = 42, b = 43))[md_i_c<0>] == 42);
        static_assert(value_of(v, at(a = 42, b = 43))[md_i_c<1>] == 43);
        expect(eq(value_of(v, at(a = 42, b = 43))[md_i_c<0>], 42));
        expect(eq(value_of(v, at(a = 42, b = 43))[md_i_c<1>], 43));
    };

    "tensor_entry_expression_derivative_wrt_tensor"_test = [] () {
        {
            static constexpr tensor T{shape<2, 2>};
            static constexpr auto expr = T[at<0, 0>()]*val<12>;
            static constexpr auto deriv = derivative_of(expr, wrt(T));
            const auto expected = linalg::tensor{shape<2, 2>, 12, 0, 0, 0};
            expect(expected == value_of(deriv, at(T = linalg::tensor{shape<2, 2>, 42, 43, 44, 45})));
            expect(expected == value_of(deriv, at(T = linalg::tensor{shape<2, 2>, 48, 49, 50, 51})));
        }
        {
            static constexpr tensor T{shape<2, 2>};
            static constexpr auto expr = T[at<1, 1>()]*val<13>;
            static constexpr auto deriv = derivative_of(expr, wrt(T));
            const auto expected = linalg::tensor{shape<2, 2>, 0, 0, 0, 13};
            expect(expected == value_of(deriv, at(T = linalg::tensor{shape<2, 2>, 42, 43, 44, 45})));
            expect(expected == value_of(deriv, at(T = linalg::tensor{shape<2, 2>, 48, 49, 50, 51})));
        }
    };

    "vector_expression_derivative_wrt_tensor"_test = [] () {
        static constexpr tensor T{shape<2, 2>};
        static constexpr auto T00 = T[at<0, 0>()]; static constexpr auto T01 = T[at<0, 1>()];
        static constexpr auto T10 = T[at<1, 0>()]; static constexpr auto T11 = T[at<1, 1>()];
        static constexpr auto T_value = linalg::tensor{shape<2, 2>, 1, 2, 3, 4};
        static constexpr auto v = vector_expression_builder<2>{}
                                    .with(T00*T01, at<0>())
                                    .with(T10*T11, at<1>())
                                    .build();
        static constexpr auto deriv = derivative_of(v, wrt(T));
        const auto dv_dT = value_of(deriv, at(T = T_value));
        expect(dv_dT[at<0>()] == linalg::tensor{shape<2, 2>, T_value[at<0, 1>()], T_value[at<0, 0>()], 0, 0});
        expect(dv_dT[at<1>()] == linalg::tensor{shape<2, 2>, 0, 0, T_value[at<1, 1>()], T_value[at<1, 0>()]});
    };

    "tensor_expression_value_with_constants"_test = [] () {
        var a;
        tensor_expression v{shape<2>, a, val<43>};
        static_assert(std::is_same_v<decltype(v[md_i_c<0>]), decltype(a)>);
        static_assert(std::is_same_v<decltype(v[md_i_c<1>]), value<43>>);
        static_assert(value_of(v, at(a = 42))[md_i_c<0>] == 42);
        static_assert(value_of(v, at(a = 42))[md_i_c<1>] == 43);
        expect(eq(value_of(v, at(a = 42))[md_i_c<0>], 42));
        expect(eq(value_of(v, at(a = 42))[md_i_c<1>], 43));
    };

    "vector_expression_builder"_test = [] () {
        var a; var b;
        constexpr auto vector = vector_expression_builder<2>{}
                                    .with(a, at<0>())
                                    .with(b, at<1>())
                                    .build();
        constexpr auto value = value_of(vector, at(a = 1, b = 2));
        static_assert(value[at<0>()] == 1);
        static_assert(value[at<1>()] == 2);
    };

    "tensor_expression_builder"_test = [] () {
        var a; var b; var c; var d;
        constexpr auto tensor = tensor_expression_builder{shape<2, 2>}
                                    .with(a, at<0, 0>())
                                    .with(b, at<0, 1>())
                                    .with(c, at<1, 0>())
                                    .with(d, at<1, 1>())
                                    .build();
        constexpr auto value = value_of(tensor, at(a = 1, b = 2, c = 3, d = 4));
        static_assert(value[at<0, 0>()] == 1);
        static_assert(value[at<0, 1>()] == 2);
        static_assert(value[at<1, 0>()] == 3);
        static_assert(value[at<1, 1>()] == 4);
    };

    "tensor_expression_builder_filled_with"_test = [] () {
        var a;
        constexpr auto tensor = tensor_expression_builder{shape<2, 2>}.filled_with(a).build();
        const auto check = [&] <typename shape, typename... T> (const tensor_expression<shape, T...>&) constexpr {
            static_assert(std::conjunction_v<std::is_same<T, decltype(a)>...>);
        };
        check(tensor);
    };

    "tensor_expression_stream"_test = [] () {
        var a; var b; var c; var d;
        auto T = tensor_expression_builder{shape<2, 2>}
                    .with(a, at<0, 0>())
                    .with(b, at<0, 1>())
                    .with(c, at<1, 0>())
                    .with(d, at<1, 1>())
                    .build();
        std::ostringstream s;
        write_to(s, T, at(a = "a", b = "b", c = "c", d = "d"));
        expect(s.str().starts_with("T<2, 2>("));
    };

    "vector_expression_stream"_test = [] () {
        var a; var b;
        auto v = vector_expression_builder<2>{}.with(a, at<0>()).with(b, at<1>()).build();
        std::ostringstream s;
        write_to(s, v, at(a = "a", b = "b"));
        expect(s.str() == "T<2>(a, b)" or s.str() == "T<2>(b, a)");
    };

    "gradient_of_scalar_expression_from_vectors"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr auto v1 = vector_expression_builder<2>{}.with(a, at<0>()).with(a, at<1>()).build();
        static constexpr auto v2 = vector_expression_builder<2>{}.with(b, at<0>()).with(b, at<1>()).build();
        static constexpr auto scalar_product = v1*v2;
        const auto grad = gradient_of(scalar_product, at(a = 1, b = 2));
        expect(eq(value_of(scalar_product, at(a = 1, b = 2)), 4));
        expect(eq(grad[a], 4));
        expect(eq(grad[b], 2));
    };

    return 0;
}
