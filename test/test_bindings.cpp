#include <utility>
#include <type_traits>

#include <xpress/bindings.hpp>

#include "testing.hpp"


struct value { int v; };

template<typename S = int>
struct symbol {
    template<typename T>
    constexpr auto operator=(T&& t) const noexcept -> xp::value_binder<symbol, T> {
        return {symbol{}, std::forward<T>(t)};
    }
};


int main() {
    using namespace xp::testing;

    "value_binder"_test = [] () {
        symbol s{};
        expect(eq((s = value{42}).get().v, 42));

        static constexpr auto v = value{42};
        constexpr auto b = (s = v);
        static_assert(b.get().v == 42);
    };

    "value_binder_by_reference"_test = [] () {
        value v{0};
        xp::value_binder binder{symbol{}, v};
        expect(eq(binder.get().v, 0));
        expect(eq(v.v, 0));

        v.v = 42;
        expect(eq(binder.get().v, 42));
        expect(eq(v.v, 42));

        binder.get() = value{44};
        expect(eq(v.v, 44));
    };

    "bindings"_test = [] () {
        symbol<int> s1{};
        symbol<char> s2{};

        constexpr xp::bindings binders{s1 = 42, s2 = 44};
        expect(eq(binders[s1], 42));
        expect(eq(binders[s2], 44));
        static_assert(binders[s1] == 42);
        static_assert(binders[s2] == 44);
    };

    "bindings_by_reference"_test = [] () {
        int v1 = 0;
        int v2 = 1;

        symbol<int> s1{};
        symbol<char> s2{};

        xp::bindings binders{s1 = v1, s2 = v2};
        v1 = 42;
        v2 = 44;
        expect(eq(binders[s1], 42));
        expect(eq(binders[s2], 44));
    };

    return 0;
}
