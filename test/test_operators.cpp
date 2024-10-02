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

    return 0;
}
