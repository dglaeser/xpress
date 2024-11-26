// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
// SPDX-License-Identifier: MIT

#include <functional>
#include <type_traits>
#include <algorithm>
#include <sstream>

#include <xpress/utils.hpp>

#include "testing.hpp"

int main() {
    using namespace xp::testing;
    using namespace xp;

    "md_shape_properties"_test = [] () {
        constexpr md_shape<2, 3> shape;
        static_assert(shape.dimensions == 2);
        static_assert(shape.count == 6);
    };

    "md_shape_equality"_test = [] () {
        constexpr md_shape<2, 3> shape;
        static_assert(shape == md_shape<2, 3>{});
        static_assert(shape != md_shape<3, 3>{});
    };

    "md_shape_is_square"_test = [] () {
        static_assert(shape<2, 2>.is_square);
        static_assert(shape<3, 3>.is_square);
        static_assert(!shape<2, 2, 2>.is_square);
        static_assert(!shape<2, 3>.is_square);
    };

    "md_shape_access"_test = [] () {
        constexpr md_shape<2, 3> shape;
        static_assert(shape.at(ic<0>) == 2);
        static_assert(shape.at(ic<1>) == 3);
    };

    "md_index_equality"_test = [] () {
        constexpr md_index<2, 3> index;
        static_assert(index == md_index<2, 3>{});
        static_assert(index != md_index<3, 3>{});
    };

    "md_index_access"_test = [] () {
        constexpr md_index<2, 3> index;
        static_assert(index.at(ic<0>) == ic<2>);
        static_assert(index.at(ic<1>) == ic<3>);
    };

    "md_index_prepended"_test = [] () {
        constexpr md_index<2, 3> index;
        static_assert(index.with_prepended(ic<0>) == md_index<0, 2, 3>{});
    };

    "md_index_appended"_test = [] () {
        constexpr md_index<2, 3> index;
        static_assert(index.with_appended(ic<0>) == md_index<2, 3, 0>{});
    };

    "md_index_as_flat_index"_test = [] () {
        static_assert(md_ic<0, 0>.as_flat_index_in(md_shape<3, 3>{}) == ic<0>);
        static_assert(md_ic<0, 1>.as_flat_index_in(md_shape<3, 3>{}) == ic<1>);
        static_assert(md_ic<0, 2>.as_flat_index_in(md_shape<3, 3>{}) == ic<2>);
        static_assert(md_ic<1, 0>.as_flat_index_in(md_shape<3, 3>{}) == ic<3>);
        static_assert(md_ic<1, 1>.as_flat_index_in(md_shape<3, 3>{}) == ic<4>);
        static_assert(md_ic<1, 2>.as_flat_index_in(md_shape<3, 3>{}) == ic<5>);
        static_assert(md_ic<2, 0>.as_flat_index_in(md_shape<3, 3>{}) == ic<6>);
        static_assert(md_ic<2, 1>.as_flat_index_in(md_shape<3, 3>{}) == ic<7>);
        static_assert(md_ic<2, 2>.as_flat_index_in(md_shape<3, 3>{}) == ic<8>);
        static_assert(md_ic<>.as_flat_index_in(md_shape<>{}) == ic<0>);
    };

    "md_index_contained_in"_test = [] () {
        static_assert(md_ic<0, 0>.is_contained_in(shape<2, 2>));
        static_assert(md_ic<0, 1>.is_contained_in(shape<2, 2>));
        static_assert(md_ic<1, 0>.is_contained_in(shape<2, 2>));
        static_assert(md_ic<1, 1>.is_contained_in(shape<2, 2>));
        static_assert(!md_ic<1, 2>.is_contained_in(shape<2, 2>));
        static_assert(!md_ic<2, 1>.is_contained_in(shape<2, 2>));
    };

    "md_index_iterator"_test = [] () {
        auto it = md_index_iterator{shape<2, 3>};
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

    "md_shape_visit"_test = [] () {
        std::array<std::array<int, 2>, 3> values{{{42, 42}, {42, 42}, {42, 42}}};
        const auto check_equal = [] (const auto& data, const auto& value) constexpr {
            return std::ranges::all_of(data, [&] (auto& row) {
                return std::ranges::all_of(row, [&] (auto& v) { return v == value; });
            });
        };
        expect(check_equal(values, 42));
        const auto duplicated = [&] (auto data) constexpr {
            visit_indices_in(shape<3, 2>, [&] (const auto& idx) constexpr noexcept {
                data[idx.at(ic<0>)][idx.at(ic<1>)] *= 2;
            });
            return data;
        };
        expect(check_equal(duplicated(values), 84));
    };

    return 0;
}
