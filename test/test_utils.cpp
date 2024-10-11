#include <functional>
#include <type_traits>
#include <algorithm>
#include <sstream>

#include <adac/utils.hpp>

#include "testing.hpp"

int main() {
    using namespace adac::testing;
    using namespace adac;

    "value_list_access"_test = [] () {
        constexpr value_list<0, 1, 2> values;
        static_assert(values.at(i_c<0>) == 0);
        static_assert(values.at(i_c<1>) == 1);
        static_assert(values.at(i_c<2>) == 2);

        static_assert(values.first() == 0);
        static_assert(values.last() == 2);
    };

    "value_list_drop_n"_test = [] () {
        constexpr value_list<0, 1, 2> values;
        constexpr auto dropped = values.template drop<1>();
        static_assert(dropped.size == 2);
        static_assert(dropped.at(i_c<0>) == 1);
        static_assert(dropped.at(i_c<1>) == 2);

        constexpr auto empty = values.template drop<3>();
        static_assert(empty.size == 0);
    };

    "value_list_crop_n"_test = [] () {
        constexpr value_list<0, 1, 2> values;
        constexpr auto cropped = values.template crop<1>();
        static_assert(cropped.size == 2);
        static_assert(cropped.at(i_c<0>) == 0);
        static_assert(cropped.at(i_c<1>) == 1);

        constexpr auto empty = values.template crop<3>();
        static_assert(empty.size == 0);
    };

    "value_list_equality"_test = [] () {
        constexpr value_list<0, 1, 2> values;
        static_assert(values == value_list<0, 1, 2>{});
    };

    "value_list_stream"_test = [] () {
        std::ostringstream s;
        s << value_list<0, 1, 2>{};
        expect(eq(s.str(), std::string{"[0,1,2,]"}));
    };

    "value_list_reduce"_test = [] () {
        constexpr value_list<0, 1, 2> values;
        static_assert(values.reduce_with(std::plus{}, 0) == 3);
        static_assert(values.reduce_with(std::multiplies{}, 1) == 0);
    };

    "value_list_concat"_test = [] () {
        constexpr value_list<0, 1, 2> values0;
        constexpr value_list<3, 4> values1;
        constexpr auto sum = values0 + values1;
        static_assert(std::is_same_v<
            std::remove_cvref_t<decltype(sum)>,
            value_list<0, 1, 2, 3, 4>
        >);
    };

    "md_shape_properties"_test = [] () {
        constexpr md_shape<2, 3> shape;
        static_assert(shape.size == 2);
        static_assert(shape.count == 6);
    };

    "md_shape_equality"_test = [] () {
        constexpr md_shape<2, 3> shape;
        static_assert(shape == md_shape<2, 3>{});
        static_assert(shape != md_shape<3, 3>{});
    };

    "md_shape_access"_test = [] () {
        constexpr md_shape<2, 3> shape;
        static_assert(shape.at(i_c<0>) == 2);
        static_assert(shape.at(i_c<1>) == 3);
    };

    "md_index_equality"_test = [] () {
        constexpr md_index<2, 3> index;
        static_assert(index == md_index<2, 3>{});
        static_assert(index != md_index<3, 3>{});
    };

    "md_index_access"_test = [] () {
        constexpr md_index<2, 3> index;
        static_assert(index.at(i_c<0>) == i_c<2>);
        static_assert(index.at(i_c<1>) == i_c<3>);
    };

    "md_index_prepended"_test = [] () {
        constexpr md_index<2, 3> index;
        static_assert(index.with_prepended(i_c<0>) == md_index<0, 2, 3>{});
    };

    "md_index_appended"_test = [] () {
        constexpr md_index<2, 3> index;
        static_assert(index.with_appended(i_c<0>) == md_index<2, 3, 0>{});
    };

    "md_index_as_flat_index"_test = [] () {
        static_assert(md_i_c<0, 0>.as_flat_index_in(md_shape<3, 3>{}) == i_c<0>);
        static_assert(md_i_c<0, 1>.as_flat_index_in(md_shape<3, 3>{}) == i_c<1>);
        static_assert(md_i_c<0, 2>.as_flat_index_in(md_shape<3, 3>{}) == i_c<2>);
        static_assert(md_i_c<1, 0>.as_flat_index_in(md_shape<3, 3>{}) == i_c<3>);
        static_assert(md_i_c<1, 1>.as_flat_index_in(md_shape<3, 3>{}) == i_c<4>);
        static_assert(md_i_c<1, 2>.as_flat_index_in(md_shape<3, 3>{}) == i_c<5>);
        static_assert(md_i_c<2, 0>.as_flat_index_in(md_shape<3, 3>{}) == i_c<6>);
        static_assert(md_i_c<2, 1>.as_flat_index_in(md_shape<3, 3>{}) == i_c<7>);
        static_assert(md_i_c<2, 2>.as_flat_index_in(md_shape<3, 3>{}) == i_c<8>);
        static_assert(md_i_c<>.as_flat_index_in(md_shape<>{}) == i_c<0>);
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
                data[idx.at(i_c<0>)][idx.at(i_c<1>)] *= 2;
            });
            return data;
        };
        expect(check_equal(duplicated(values), 84));
    };

    return 0;
}
