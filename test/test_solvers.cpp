#include <xpress/xp.hpp>
#include <xpress/solvers/newton.hpp>

#include "testing.hpp"

int main() {
    using namespace xp;
    using namespace xp::solvers;
    using namespace xp::testing;

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

    "newton_solver_vector_equation"_test = [] () {
        var a;
        var b;
        constexpr auto eq_system = vector_expression_builder<2>{}
                                    .with(a*a - val<1.0>, at<0>())
                                    .with(b*b - val<1.0>, at<1>())
                                    .build();
        auto solution = solvers::newton{{
            .threshold = 1e-6,
            .max_iterations = 20
        }}.find_root_of(eq_system, starting_from(a = 3.0, b = 4.0));
        expect(solution.has_value());
        expect(fuzzy_eq((*solution)[a], 1.0));
        expect(fuzzy_eq((*solution)[b], 1.0));
    };

    return 0;
}
