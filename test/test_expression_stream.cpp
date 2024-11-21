#include <string>
#include <sstream>

#include <xpress/symbols.hpp>
#include <xpress/operators.hpp>

#include "testing.hpp"

int main() {
    using namespace xp;
    using namespace xp::testing;

    "addition_stream"_test = [] () {
        var a;
        var b;
        var c;
        std::ostringstream out;
        write_to(out, a + (b + c), with(a = "a", b = "b", c = "c"));
        expect(eq(out.str(), std::string{"a + b + c"}));
    };

    "mixed_arithmetic_stream"_test = [] () {
        var a;
        var b;
        var c;
        auto expression = (a + b)/c + (b + c)*a;
        std::ostringstream out;
        write_to(out, expression, with(a = "a", b = "b", c = "c"));
        expect(eq(out.str(), std::string{"(a + b)/c + (b + c)*a"}));
    };

    "mixed_nested_arithmetic_stream"_test = [] () {
        var a;
        var b;
        var c;
        auto expression = a*b/c + (b + c)*a;
        std::ostringstream out;
        write_to(out, expression, with(a = "a", b = "b", c = "c"));
        expect(eq(out.str(), std::string{"(a*b)/c + (b + c)*a"}));
    };

    "add_operator_same_operand"_test = [] () {
        static constexpr let a;
        constexpr auto added = a + a;
        std::ostringstream out;
        write_to(out, added, with(a = "a"));
        expect(eq(out.str(), std::string{"2*a"}));
    };

    return 0;
}
