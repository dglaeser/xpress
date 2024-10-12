#include <adac/adac.hpp>
#include <adac/solvers/newton.hpp>

#include "testing.hpp"

int main() {
    using namespace adac;
    using namespace adac::solvers;
    using namespace adac::testing;

    "netwon_solver_constexpr"_test = [] () {
        var a;
        constexpr auto solution = solvers::newton{{
            .threshold = 1e-6,
            .max_iterations = 20
        }}.find_root_of(a*a - val<1.0>, starting_from(a = 3));
        static_assert(solution.has_value());
        static_assert(fuzzy_eq((*solution)[a], 1.0));
    };

    "newton_solver_at_runtime"_test = [] () {
        var a;
        expect(fuzzy_eq(solvers::newton{{
            .threshold = 1e-6,
            .max_iterations = 20
        }}.find_root_of(a*a - val<1.0>, starting_from(a = 3)).value()[a], 1.0));
    };

    "newton_solver_failure"_test = [] () {
        var a;
        expect(!solvers::newton{{
            .threshold = 1e-6,
            .max_iterations = 1
        }}.find_root_of(a*a - val<1.0>, starting_from(a = 3)).has_value());
    };

    return 0;
}
