#include <adac/utils.hpp>

#include "testing.hpp"

int main() {
    using namespace adac::testing;

    "always_false"_test = [] () {
        static_assert(adac::always_false<int>::value == false);
        expect(eq(adac::always_false<int>::value, false));
    };

    "is_less"_test = [] () {
        static_assert(adac::is_less_v<0, 1> == true);
        static_assert(adac::is_less_v<1, 1> == false);
        expect(eq(adac::is_less_v<0, 1>, true));
        expect(eq(adac::is_less_v<1, 1>, false));
    };

    "is_equal"_test = [] () {
        static_assert(adac::is_equal_v<1, 1> == true);
        static_assert(adac::is_equal_v<0, 1> == false);
        expect(eq(adac::is_equal_v<1, 1>, true));
        expect(eq(adac::is_equal_v<0, 1>, false));
    };

    "is_less_equal"_test = [] () {
        static_assert(adac::is_less_equal_v<1, 1> == true);
        static_assert(adac::is_less_equal_v<0, 1> == true);
        static_assert(adac::is_less_equal_v<2, 1> == false);
        expect(eq(adac::is_less_equal_v<1, 1>, true));
        expect(eq(adac::is_less_equal_v<0, 1>, true));
        expect(eq(adac::is_less_equal_v<2, 1>, false));
    };

    return 0;
}
