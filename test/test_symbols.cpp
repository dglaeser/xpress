#include <adac/symbols.hpp>

#include "testing.hpp"

int main() {
    using namespace adac::testing;
    using namespace adac;
    using namespace adac::dtype;

    "unit_value"_test = [] () {
        static_assert(traits::is_unit_value_v<value<1>>);
        static_assert(traits::is_unit_value_v<value<1.0>>);
    };

    "zero_value"_test = [] () {
        static_assert(traits::is_zero_value_v<value<0>>);
        static_assert(traits::is_zero_value_v<value<0.0>>);
    };

    "let_var_symbol_concept"_test = [] () {
        let a;
        var b;
        value<42> c;
        static_assert(concepts::symbol<decltype(a)>);
        static_assert(concepts::symbol<decltype(b)>);
        static_assert(!concepts::symbol<decltype(c)>);
    };

    "let_var_value_variable_concept"_test = [] () {
        var a;
        let b;
        value<42> c;
        static_assert(concepts::symbol<decltype(a)>);
        static_assert(!concepts::variable<decltype(b)>);
        static_assert(!concepts::variable<decltype(c)>);
    };

    "let_var_value_nodes_of"_test = [] () {
        let a;
        var b;
        value<42> c;
        static_assert(std::is_same_v<
            typename traits::nodes_of<decltype(a)>::type,
            type_list<decltype(a)>
        >);
        static_assert(std::is_same_v<
            typename traits::nodes_of<decltype(b)>::type,
            type_list<decltype(b)>
        >);
        static_assert(std::is_same_v<
            typename traits::nodes_of<decltype(c)>::type,
            type_list<decltype(c)>
        >);
    };


    "let_var_eval"_test = [] () {
        static constexpr let a;
        static constexpr var b;

        static_assert(evaluate(a, at(a = 5)) == 5);
        static_assert(evaluate(a, at(a = 5.0)) == 5.0);
        expect(eq(evaluate(a, at(a = 5)), 5));
        expect(eq(evaluate(a, at(a = 5.0)), 5.0));

        constexpr auto da_da = differentiate(a, wrt(a));
        constexpr auto da_db = differentiate(a, wrt(b));
        static_assert(evaluate(da_da, at(a=2.0)) == 1);
        static_assert(evaluate(da_db, at(a=2.0)) == 0);
        expect(eq(evaluate(da_da, at(a=2.0)), 1));
        expect(eq(evaluate(da_db, at(a=2.0)), 0));
    };

    return 0;
}
