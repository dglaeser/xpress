#include <adac/dtype.hpp>

#include "testing.hpp"

template<typename T1, typename T2>
inline constexpr bool verify_same = std::is_same_v<T1, T2>;

int main() {
    using namespace adac::testing;
    using namespace adac::dtype;

    static_assert(verify_same<common_dtype_t<integral, real>, real>);
    static_assert(verify_same<common_dtype_t<real, integral>, real>);

    static_assert(verify_same<common_dtype_t<integral, integral>, integral>);
    static_assert(verify_same<common_dtype_t<real, real>, real>);

    static_assert(verify_same<common_dtype_t<integral, integral, integral, any>, any>);
    static_assert(verify_same<common_dtype_t<integral, integral, integral, real>, real>);

    return 0;
}