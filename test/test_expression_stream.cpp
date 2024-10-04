#include <string>
#include <sstream>

#include <adac/symbols.hpp>
#include <adac/operators.hpp>

#include "testing.hpp"

int main() {
    using namespace adac;
    using namespace adac::testing;

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

    return 0;
}