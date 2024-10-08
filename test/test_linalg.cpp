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

    "md_index_as_flat"_test = [] () {
        static_assert(md_index<1, 0>::as_flat_index_in(shape<2, 2>{}) == adac::index<2>);
        static_assert(md_index<1, 0>::as_flat_index_in(shape<2, 3>{}) == adac::index<3>);
        static_assert(md_index<1, 1>::as_flat_index_in(shape<2, 3>{}) == adac::index<4>);
    };

    "md_index_iterator"_test = [] () {
        auto it = md_index_iterator{shape<2, 3>{}};
        static_assert(*it == md_index<0, 0>{});
        static_assert(it.is_incrementable());

        auto inc1 = it.incremented();
        static_assert(*inc1 == md_index<0, 1>{});
        static_assert(inc1.is_incrementable());

        auto inc2 = inc1.incremented();
        static_assert(*inc2 == md_index<0, 2>{});
        static_assert(inc2.is_incrementable());

        auto inc3 = inc2.incremented();
        static_assert(*inc3 == md_index<1, 0>{});
        static_assert(inc3.is_incrementable());

        auto inc4 = inc3.incremented();
        static_assert(*inc4 == md_index<1, 1>{});
        static_assert(inc4.is_incrementable());

        auto inc5 = inc4.incremented();
        static_assert(*inc5 == md_index<1, 2>{});
        static_assert(!inc5.is_incrementable());
    };

    return 0;
}
