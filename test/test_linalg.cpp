#include <array>

#include <adac/linalg.hpp>

#include "testing.hpp"


struct indexable {
    constexpr double operator[](std::size_t) const noexcept {
        return 1.0;
    }

    static constexpr std::size_t size() noexcept {
        return 1;
    }
};

int main() {
    using namespace adac::testing;
    using namespace adac::linalg;

    static_assert(traits::is_indexable<std::array<double, 2>>::value);
    static_assert(traits::is_indexable<indexable>::value);
    static_assert(!traits::is_indexable<int>::value);

    static_assert(traits::size_of_v<std::array<double, 2>> == 2);
    static_assert(traits::size_of_v<std::array<double, 3>> == 3);
    static_assert(traits::size_of_v<indexable> == 1);

    static_assert(traits::shape_of_t<std::array<double, 2>>{} == shape<2>{});
    static_assert(traits::shape_of_t<std::array<std::array<int, 3>, 2>>{} == shape<2, 3>{});
    static_assert(traits::shape_of_t<std::array<indexable, 2>>{} == shape<2, 1>{});

    return 0;
}
