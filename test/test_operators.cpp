#include <adac/symbols.hpp>
#include <adac/operators.hpp>

#include "testing.hpp"

int main() {
    using namespace adac;
    using namespace adac::testing;

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

    return 0;
}
