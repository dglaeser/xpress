#include <sstream>
#include <type_traits>

#include <adac/tensor.hpp>

#include "testing.hpp"

int main() {
    using namespace adac::testing;
    using namespace adac;

    "tensor_uniqueness_test"_test = [] () {
        tensor t1{shape<2, 3>};
        tensor t2{shape<2, 3>};
        static_assert(!std::is_same_v<decltype(t1), decltype(t2)>);
    };

    "tensor_stream"_test = [] () {
        tensor T{shape<2, 3>};
        std::ostringstream s;
        write_to(s, T, at(T = "T<2, 3>"));
        expect(eq(s.str(), std::string{"T<2, 3>"}));
    };

    return 0;
}
