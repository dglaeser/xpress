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
        constexpr auto derivative = differentiate(added, wrt(a));
        static_assert(evaluate(derivative) == 1);
        expect(eq(evaluate(derivative), 1));
    };

    return 0;
}
