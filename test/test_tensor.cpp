#include <array>
#include <utility>
#include <algorithm>
#include <type_traits>

#include <adac/operators.hpp>
#include <adac/tensor.hpp>

#include "testing.hpp"


template<typename T, std::size_t r, std::size_t c>
class matrix {
    constexpr matrix(std::array<std::array<T, c>, r>&& data)
    : _data{std::move(data)}
    {}

 public:
    static constexpr std::size_t rows = r;
    static constexpr std::size_t cols = c;
    static constexpr std::size_t size = rows;

    constexpr matrix() = default;

    template<typename self>
    constexpr decltype(auto) operator[](this self&& s, std::size_t i) noexcept {
        return s._data[i];
    }

    constexpr matrix operator*(T factor) const noexcept {
        auto new_data = _data;
        std::ranges::for_each(new_data, [&] (auto& row) {
            std::ranges::for_each(row, [&] (auto& value) {
                value *= factor;
            });
        });
        return matrix{std::move(new_data)};
    }

    constexpr bool operator==(const matrix& other) const noexcept {
        for (std::size_t i = 0; i < r; ++i)
            for (std::size_t j = 0; j < c; ++j)
                if (_data[i][j] != other._data[i][j])
                    return false;
        return true;
    }

 private:
    std::array<std::array<T, c>, r> _data;
};


int main() {
    using namespace adac::testing;
    using namespace adac;

    "tensor_uniqueness_test"_test = [] () {
        tensor t1{shape<2, 3>};
        tensor t2{shape<2, 3>};
        static_assert(!std::is_same_v<decltype(t1), decltype(t2)>);
    };

    "tensor_value"_test = [] () {
        matrix<int, 2, 2> m{};
        tensor t{shape<2, 2>};
        static_assert(std::is_same_v<decltype(value_of(t, at(t = m))), const matrix<int, 2, 2>&>);
    };

    "tensor_times_scalar"_test = [] () {
        matrix<int, 2, 2> m;
        m[0][0] = 1; m[0][1] = 2;
        m[1][0] = 3; m[1][1] = 4;

        tensor t{shape<2, 2>};
        auto expression = t*val<2>;
        auto v = value_of(expression, at(t = m));
        expect(v == m*2);
    };

    "tensor_stream"_test = [] () {
        tensor T{shape<2, 3>};
        std::ostringstream s;
        write_to(s, T, at(T = "T<2, 3>"));
        expect(eq(s.str(), std::string{"T<2, 3>"}));
    };

    return 0;
}
