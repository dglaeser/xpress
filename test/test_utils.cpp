#include <functional>
#include <type_traits>
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

    return 0;
}
