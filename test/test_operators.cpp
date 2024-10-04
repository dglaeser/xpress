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
        static_assert(evaluate(added, at(a = 1, b = 2)) == 3);
        expect(eq(evaluate(added, at(a = 1, b = 2)), 3));
    };

    "add_operator_derivative"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto added = a + b;
        constexpr auto d_da = differentiate(added, wrt(a));
        constexpr auto d_db = differentiate(added, wrt(a));
        static_assert(evaluate(d_da) == 1);
        static_assert(evaluate(d_db) == 1);
        expect(eq(evaluate(d_da), 1));
        expect(eq(evaluate(d_db), 1));
    };

    "subtract_operator_value"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto subtracted = a - b;
        static_assert(evaluate(subtracted, at(a = 1, b = 2)) == -1);
        expect(eq(evaluate(subtracted, at(a = 1, b = 2)), -1));
    };

    "subtract_operator_derivative"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto subtracted = a - b;
        constexpr auto d_da = differentiate(subtracted, wrt(a));
        constexpr auto d_db = differentiate(subtracted, wrt(b));
        static_assert(evaluate(d_da) == 1);
        static_assert(evaluate(d_db) == -1);
        expect(eq(evaluate(d_da), 1));
        expect(eq(evaluate(d_db), -1));
    };

    "multiply_operator_value"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto multiplied = a*b;
        static_assert(evaluate(multiplied, at(a = 2, b = 42)) == 84);
        static_assert(evaluate(multiplied, at(a = 43, b = 2)) == 86);
        expect(eq(evaluate(multiplied, at(a = 2, b = 42)), 84));
        expect(eq(evaluate(multiplied, at(a = 43, b = 2)), 86));
    };

    "multiply_operator_derivative"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto multiplied = a*b;
        constexpr auto d_da = differentiate(multiplied, wrt(a));
        constexpr auto d_db = differentiate(multiplied, wrt(b));
        static_assert(evaluate(d_da, at(a = 1, b = 42)) == 42);
        static_assert(evaluate(d_db, at(a = 43, b = 1)) == 43);
        expect(eq(evaluate(d_da, at(a = 1, b = 42)), 42));
        expect(eq(evaluate(d_db, at(a = 43, b = 1)), 43));
    };

    "division_operator_value"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto divided = a/b;
        static_assert(evaluate(divided, at(a = 42, b = 2)) == 21);
        expect(eq(evaluate(divided, at(a = 42, b = 2)), 21));
    };

    "division_operator_derivative"_test = [] () {
        static constexpr let a;
        static constexpr var b;
        constexpr auto divided = a/b;
        constexpr auto d_da = differentiate(divided, wrt(a));
        constexpr auto d_db = differentiate(divided, wrt(b));
        static_assert(evaluate(d_da, at(a = 1., b = 42.)) == 1.0/42.0);
        static_assert(evaluate(d_db, at(a = 2., b = 42.)) == 0.0 - 1.0*2.0/(42.0*42.0));
        expect(eq(evaluate(d_da, at(a = 1., b = 42.)), 1.0/42.0));
        expect(eq(evaluate(d_db, at(a = 2., b = 42.)), 0.0 - 1.0*2.0/(42.0*42.0)));
    };

    "expression_nodes_of"_test = [] () {
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

    "expression_unique_nodes_of"_test = [] () {
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

    "expression_dtype_with_any"_test = [] () {
        let<dtype::real> a;
        let<dtype::integral> b;
        let<dtype::any> c;
        auto expr = (a + b)*c;
        static_assert(std::is_same_v<dtype_of_t<decltype(expr)>, dtype::any>);
    };

    "expression_dtype_yielding_real"_test = [] () {
        let<dtype::real> a;
        let<dtype::integral> b;
        let<dtype::integral> c;
        auto expr = (a + b)*c;
        static_assert(std::is_same_v<dtype_of_t<decltype(expr)>, dtype::real>);
    };

    return 0;
}
