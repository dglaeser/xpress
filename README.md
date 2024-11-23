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
    // Since we want to print the expression, we simply bind names to the
    // variables here.
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

## Evaluating expressions

As mentioned, for evaluations we need to bind values to the variables of the expression.
All symbols and expression types allow to bind values to them via `operator=`,
and we may use the `at` function to combine the results into a single data structure (you may also use the `with` function - which is the same as `at` - depending on your preferences).
Finally, `value_of` evaluates the expression at the provided values.

```cpp <!-- {{xpress-eval-snippet}} -->
var a;
var b;
auto expr = a*a + b;
std::println("expr = {}", value_of(expr, at(a = 2.0, b = 3.0)));
```

You can also bind values to sub-expressions, which may speed up the computations:

```cpp <!-- {{xpress-subexpr-snippet}} -->
var a;
var b;
auto arg = a*a + b;
auto expr = log(arg) + arg*arg;
auto arg_value = value_of(arg, at(a = 2.0, b = 3.0));
std::println("expr = {}", value_of(expr, with(arg = arg_value)));
```

Since each `var` and each expressions are unique types, compilation will fail
if you don't provide all the values required for an evaluation. By providing
a value for `arg`, however, provision of values for the leaf symbols `a` and `b`
becomes obsolete.

You can also use `value_of` without providing any values for the symbols, in which
case you receive an evaluator object, which you can store and subsequently invoke to get the values:

```cpp <!-- {{xpress-exprevaluator-snippet}} -->
var a;
var b;
auto f = value_of(a*log(b));
std::println("f = {}", f(a = 2.0, b = 3.0));
```

Finally, as we have seen before, you can create a `bound_expression` from an expression and simply extract the value:

```cpp <!-- {{xpress-boundexpr-snippet}} -->
var a;
var b;
auto bound_expr = (a*b + b*b).with(a = 2.0, b = 3.0);
std::println("expr = {}", bound_expr.value());
```

Note that the primary use of `bound_expression` is to print an expression to the terminal (or to create a `std::string`), as we have seen before.

## Deriving expressions

Using `derivative_of`, you receive a new expression that is the derivative of the provided one with respect to the provided variables.

```cpp <!-- {{xpress-deriv-snippet}} -->
var a;
var b;
auto de_da = derivative_of(a*log(b), wrt(a));
std::println("de_da = {}", de_da.with(a = "a", b = "b"));
```

To direclty compute the value of the derivative, you may provide values for the symbols of the expression:

```cpp <!-- {{xpress-deriveval-snippet}} -->
var a;
var b;
auto de_da = derivative_of(a*log(b), wrt(a), at(a = 42.0, b = 1.0));
std::println("de_da = {}", de_da);
```

As a side note, since `a` does not appear in the above derivative anymore, we would not have to provide a value for it.
However, usually you don't know the expression of the derivative in advance, and thus, providing values for all symbols should be the default.

To get the derivatives with respect to multiple variables at once, you may use the `derivatives_of` and access the derivatives individually on the
resulting data structure:

```cpp <!-- {{xpress-derivs-snippet}} -->
var a;
var b;
auto derivs = derivatives_of(a*log(b), wrt(a, b));
std::println("de_da = {}", derivs.wrt(a).with(a = "a", b = "b"));
std::println("de_db = {}", derivs.wrt(b).with(a = "a", b = "b"));
```

Of course you can also directly compute the values of the derivatives.
The returned object allows you to extract the derivatives with respect
to an individual variable by accessing it with that variable.

```cpp <!-- {{xpress-derivseval-snippet}} -->
var a;
var b;
auto derivs = derivatives_of(a*log(b), wrt(a, b), at(a = 1.0, b = 2.0));
std::println("de_da = {}", derivs[a]);
std::println("de_db = {}", derivs[b]);
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
