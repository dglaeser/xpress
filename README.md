`xpress` is a header-only C++ template library that let's you write strongly-typed mathematical expressions,
and, for instance, derive them with respect to one or more variables at compile-time. To use the library,
simply add this repository as git submodule to yours, or, if your project uses `cmake`, you may use `FetchContent`
to pull it upon configuration. Alernatively, you can install the library locally on your system. See below for examples
on the latter two approaches.

## Prerequisites

- `cmake` (> 3.18)
- Compiler with support for `c++-23`

## Quick start

The following is a minimal example application that uses this library
to define an expression and print it to the terminal.

```cpp <!-- {{xpress-minimal-main}} -->
// my_app.cpp
#include <print>
#include <xpress/xp.hpp>

int main() {
    using namespace xp;
    // the variables of our expression
    var a;
    var b;

    // the actual expression
    constexpr auto expr = a + b;

    // For an evaluation of `expr` we need to provide values for the variables.
    // `expr.with` is one way to do so, which yields a `bound_expression`
    // that can be inserted to an ostream or passed to std::print.
    // Since we want to print the expression, we simply bind strings
    // (actually, here `const char*`) to the variables.
    std::print("{}", expr.with(a = "a", b = "b"));  // prints "a + b"

    return 0;
}
```

With the following `CMakeLists.txt` file being present in the same directory as the above main file:

```cmake <!-- {{xpress-minimal-cmakelists}} -->
# CMakeLists.txt
cmake_minimum_required(VERSION 3.18)
project(xpress_example)

set(XPRESS_BUILD_TESTS OFF)
include(FetchContent)
FetchContent_Declare(
    xpress
    GIT_REPOSITORY https://github.com/dglaeser/xpress.git
    GIT_TAG main
)
FetchContent_MakeAvailable(xpress)

add_executable(my_app my_app.cpp)
target_link_libraries(my_app PRIVATE xpress::xpress)
```

you can compile and run this code with the following sequence of commands
(this assumes that your default compilers are recent enough; otherwise you have
to pass the `cmake` flags `-DCMAKE_C_COMPILER` and `-DCMAKE_CXX_COMPILER` to have `cmake` use suitable compilers):

```bash <!-- {{xpress-minimal-bash}} -->
cmake -B build
cmake --build build
cd build && ./my_app
```

## Installation

You may use `cmake` to install the library on your system, for instance, with the following sequence of commands.

```bash <!-- {{xpress-install-instructions}} -->
git clone --recursive https://github.com/dglaeser/xpress.git
cd xpress
cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/xpress-install -DXPRESS_BUILD_TESTS=OFF -B build
cmake --build build
cmake --install build
```

Afterwards, you can use `find_package` to let `cmake` find the library, and link against the `xpress::xpress` target as described above.
