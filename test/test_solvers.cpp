#include <adac/adac.hpp>
#include <adac/solvers/newton.hpp>

#include "testing.hpp"

int main() {
    using namespace adac;
    using namespace adac::solvers;
    using namespace adac::testing;

    "netwon_solver"_test = [] () {
        var a;
        constexpr auto solution = solvers::newton{{
            .threshold = 1e-6,
            .max_iterations = 20
        }}.find_root_of(a*a - val<1.0>, starting_from(a = 3));
        static_assert(fuzzy_eq(solution[a], 1.0));
        expect(fuzzy_eq(solution[a], 1.0));
    };

    return 0;
}
