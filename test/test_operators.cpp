#include <adac/symbols.hpp>
#include <adac/operators.hpp>

#include "testing.hpp"

int main() {
    using namespace adac;
    using namespace adac::testing;

    "value_operators"_test = [] {
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(val<0> + val<1>)>, value<1>>);
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(val<0> - val<1>)>, value<-1>>);
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(val<2> * val<3>)>, value<6>>);
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(val<6> / val<3>)>, value<2>>);
        static_assert(std::is_same_v<std::remove_cvref_t<decltype(val<1> / val<2.0>)>, value<0.5>>);
    };

    "add_operator_value"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto added = a + b;

        static_assert(value_of(added, at(a = 1, b = 2)) == 3);
        expect(eq(value_of(added, at(a = 1, b = 2)), 3));

        // alternative syntax via evaluator
        static_assert(value_of(added).at(a = 1, b = 2) == 3);
        expect(eq(value_of(added).at(a = 1, b = 2), 3));
    };

    "add_operator_derivative"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto added = a + b;
        constexpr auto d_da = derivative_of(added, wrt(a));
        constexpr auto d_db = derivative_of(added, wrt(a));
        static_assert(value_of(d_da, at()) == 1);
        static_assert(value_of(d_db, at()) == 1);
        expect(eq(value_of(d_da, at()), 1));
        expect(eq(value_of(d_db, at()), 1));
    };

    "subtract_operator_value"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto subtracted = a - b;
        static_assert(value_of(subtracted, at(a = 1, b = 2)) == -1);
        expect(eq(value_of(subtracted, at(a = 1, b = 2)), -1));
    };

    "subtract_operator_derivative"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto subtracted = a - b;
        constexpr auto d_da = derivative_of(subtracted, wrt(a));
        constexpr auto d_db = derivative_of(subtracted, wrt(b));
        static_assert(value_of(d_da, at()) == 1);
        static_assert(value_of(d_db, at()) == -1);
        expect(eq(value_of(d_da, at()), 1));
        expect(eq(value_of(d_db, at()), -1));
    };

    "multiply_operator_value"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto multiplied = a*b;
        static_assert(value_of(multiplied, at(a = 2, b = 42)) == 84);
        static_assert(value_of(multiplied, at(a = 43, b = 2)) == 86);
        expect(eq(value_of(multiplied, at(a = 2, b = 42)), 84));
        expect(eq(value_of(multiplied, at(a = 43, b = 2)), 86));
    };

    "multiply_operator_derivative"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto multiplied = a*b;
        constexpr auto d_da = derivative_of(multiplied, wrt(a));
        constexpr auto d_db = derivative_of(multiplied, wrt(b));
        static_assert(value_of(d_da, at(a = 1, b = 42)) == 42);
        static_assert(value_of(d_db, at(a = 43, b = 1)) == 43);
        expect(eq(value_of(d_da, at(a = 1, b = 42)), 42));
        expect(eq(value_of(d_db, at(a = 43, b = 1)), 43));
    };

    "division_operator_value"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto divided = a/b;
        static_assert(value_of(divided, at(a = 42, b = 2)) == 21);
        expect(eq(value_of(divided, at(a = 42, b = 2)), 21));
    };

    "division_operator_derivative"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto divided = a/b;
        constexpr auto d_da = derivative_of(divided, wrt(a));
        constexpr auto d_db = derivative_of(divided, wrt(b));
        static_assert(value_of(d_da, at(a = 1., b = 42.)) == 1.0/42.0);
        static_assert(value_of(d_db, at(a = 2., b = 42.)) == 0.0 - 1.0*2.0/(42.0*42.0));
        expect(eq(value_of(d_da, at(a = 1., b = 42.)), 1.0/42.0));
        expect(eq(value_of(d_db, at(a = 2., b = 42.)), 0.0 - 1.0*2.0/(42.0*42.0)));
    };

    "operation_derivative_wrt_expression"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        static constexpr auto sum = a + b;
        static constexpr auto result = val<42>*sum;
        constexpr auto d_dsum = derivative_of(result, wrt(sum));
        static_assert(value_of(d_dsum, at()) == 42);
        expect(eq(value_of(d_dsum, at()), 42));
    };

    "operation_direct_derivative_evaluation"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        static constexpr auto sum = a + b;
        static constexpr auto result = val<42>*sum;
        static_assert(derivative_of(result, wrt(sum), at()) == 42);
        expect(eq(derivative_of(result, wrt(sum), at()), 42));
    };

    "operation_derivative_wrt_multiple_vars"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        static constexpr var c;
        static constexpr auto expression = val<42>*(a + val<2>*b) + c;
        static constexpr auto derivs = derivatives_of(expression, wrt(a, b, c));

        // evaluation via value_of interface
        static_assert(value_of(derivs.wrt(a), at(a = 0, b = 0, c = 0)) == 42);
        static_assert(value_of(derivs.wrt(b), at(a = 0, b = 0, c = 0)) == 84);
        static_assert(value_of(derivs.wrt(c), at(a = 0, b = 0, c = 0)) == 1);
        expect(eq(value_of(derivs.wrt(a), at(a = 0, b = 0, c = 0)), 42));
        expect(eq(value_of(derivs.wrt(b), at(a = 0, b = 0, c = 0)), 84));
        expect(eq(value_of(derivs.wrt(c), at(a = 0, b = 0, c = 0)), 1));

        // simultaneous evaluation and value extraction
        static_assert(derivs.at(a = 0, b = 0, c = 0)[a] == 42);
        static_assert(derivs.at(a = 0, b = 0, c = 0)[b] == 84);
        static_assert(derivs.at(a = 0, b = 0, c = 0)[c] == 1);
        expect(eq(derivs.at(a = 0, b = 0, c = 0)[a], 42));
        expect(eq(derivs.at(a = 0, b = 0, c = 0)[b], 84));
        expect(eq(derivs.at(a = 0, b = 0, c = 0)[c], 1));
    };

    "operation_direct_derivatives_evaluation"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        static constexpr var c;
        static constexpr auto expression = val<42>*(a + val<2>*b) + c;
        static constexpr auto derivs = derivatives_of(expression, wrt(a, b, c), at(a = 0, b = 0, c = 0));
        static_assert(derivs[a] == 42);
        static_assert(derivs[b] == 84);
        static_assert(derivs[c] == 1);
        expect(eq(derivs[a], 42));
        expect(eq(derivs[b], 84));
        expect(eq(derivs[c], 1));
    };

    "operation_gradient"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        constexpr auto sum = a + b*val<42>;
        constexpr auto gradient = gradient_of(sum);

        static_assert(value_of(gradient.wrt(a), at()) == 1);
        static_assert(value_of(gradient.wrt(b), at()) == 42);
        expect(eq(value_of(gradient.wrt(a), at()), 1));
        expect(eq(value_of(gradient.wrt(b), at()), 42));

        // direct evaluation on the return type
        static_assert(gradient.at()[a] == 1);
        static_assert(gradient.at()[b] == 42);
        expect(eq(gradient.at()[a], 1));
        expect(eq(gradient.at()[b], 42));

        // direct evaluation via free function
        static_assert(gradient_of(sum, at())[a] == 1);
        static_assert(gradient_of(sum, at())[b] == 42);
        expect(eq(gradient_of(sum, at())[a], 1));
        expect(eq(gradient_of(sum, at())[b], 42));
    };

    "operation_nodes_of"_test = [] () {
        using namespace adac::traits;

        let a;
        var b;
        var c;
        auto sum = a + b;
        auto c_times_sum = c*sum;
        auto result = c_times_sum*val<42>;

        using nodes = nodes_of_t<decltype(result)>;
        static_assert(type_list_size_v<nodes> == 7);
        static_assert(is_any_of_v<decltype(a), nodes>);
        static_assert(is_any_of_v<decltype(b), nodes>);
        static_assert(is_any_of_v<decltype(c), nodes>);
        static_assert(is_any_of_v<decltype(sum), nodes>);
        static_assert(is_any_of_v<decltype(c_times_sum), nodes>);
        static_assert(is_any_of_v<decltype(result), nodes>);
        static_assert(is_any_of_v<value<42>, nodes>);

        using leafs = leaf_nodes_of_t<decltype(result)>;
        static_assert(type_list_size_v<leafs> == 4);
        static_assert(is_any_of_v<decltype(a), leafs>);
        static_assert(is_any_of_v<decltype(b), leafs>);
        static_assert(is_any_of_v<decltype(c), leafs>);
        static_assert(is_any_of_v<value<42>, leafs>);

        using composites = composite_nodes_of_t<decltype(result)>;
        static_assert(type_list_size_v<composites> == 3);
        static_assert(is_any_of_v<decltype(sum), composites>);
        static_assert(is_any_of_v<decltype(c_times_sum), composites>);
        static_assert(is_any_of_v<decltype(result), composites>);
    };

    "operation_unique_nodes_of"_test = [] () {
        using namespace adac::traits;

        let a;
        var b;
        auto sum_1 = a + b;
        auto sum_2 = b + a;
        auto expr = sum_1 + sum_2;

        using nodes = nodes_of_t<decltype(expr)>;
        static_assert(type_list_size_v<nodes> == 7);
        static_assert(is_any_of_v<decltype(a), nodes>);
        static_assert(is_any_of_v<decltype(b), nodes>);
        static_assert(is_any_of_v<decltype(sum_1), nodes>);
        static_assert(is_any_of_v<decltype(sum_2), nodes>);
        static_assert(is_any_of_v<decltype(expr), nodes>);

        // duplicates of a & b should disappear
        using made_unique = unique_types_t<nodes>;
        static_assert(type_list_size_v<made_unique> == 5);
        static_assert(is_any_of_v<decltype(a), made_unique>);
        static_assert(is_any_of_v<decltype(b), made_unique>);
        static_assert(is_any_of_v<decltype(sum_1), made_unique>);
        static_assert(is_any_of_v<decltype(sum_2), made_unique>);
        static_assert(is_any_of_v<decltype(expr), made_unique>);

        // sum_1 & sum_2 should be identified as equal nodes
        using unique_nodes = unique_nodes_of_t<decltype(expr)>;
        static_assert(type_list_size_v<unique_nodes> == 4);
        static_assert(is_any_of_v<decltype(a), unique_nodes>);
        static_assert(is_any_of_v<decltype(b), unique_nodes>);
        static_assert(is_any_of_v<decltype(expr), unique_nodes>);

        using unique_leafs = unique_leaf_nodes_of_t<decltype(expr)>;
        static_assert(type_list_size_v<unique_leafs> == 2);
        static_assert(is_any_of_v<decltype(a), unique_leafs>);
        static_assert(is_any_of_v<decltype(b), unique_leafs>);

        using unique_composites = unique_composite_nodes_of_t<decltype(expr)>;
        static_assert(type_list_size_v<unique_leafs> == 2);
        static_assert(is_any_of_v<decltype(sum_1), made_unique> || is_any_of_v<decltype(sum_2), made_unique>);
        static_assert(is_any_of_v<decltype(expr), made_unique>);
    };

    "operation_symbols_variables_of"_test = [] () {
        using namespace adac::traits;

        let a;
        var b;
        auto sum_1 = a + b;
        auto sum_2 = b + a;
        auto expr = sum_1 + sum_2*val<42>;

        using symbols = symbols_of_t<decltype(expr)>;
        static_assert(type_list_size_v<symbols> == 2);
        static_assert(is_any_of_v<decltype(a), symbols>);
        static_assert(is_any_of_v<decltype(b), symbols>);

        using variables = variables_of_t<decltype(expr)>;
        static_assert(type_list_size_v<variables> == 1);
        static_assert(is_any_of_v<decltype(b), variables>);
    };

    "operation_dtype_with_any"_test = [] () {
        let<dtype::real> a;
        let<dtype::integral> b;
        let<dtype::any> c;
        auto expr = (a + b)*c;
        static_assert(std::is_same_v<typename decltype(expr)::dtype, dtype::any>);
    };

    "operation_dtype_yielding_real"_test = [] () {
        let<dtype::real> a;
        let<dtype::integral> b;
        let<dtype::integral> c;
        auto expr = (a + b)*c;
        static_assert(std::is_same_v<typename decltype(expr)::dtype, dtype::real>);
    };

    return 0;
}
