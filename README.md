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

Alternatively, you can install the library on your system, for instance, with the following sequence of commands.

```bash <!-- {{xpress-install-instructions}} -->
git clone --recursive https://github.com/dglaeser/xpress.git
cd xpress
cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/xpress-install -DXPRESS_BUILD_TESTS=OFF -B build
cmake --build build
cmake --install build
```

Afterwards, you can use `find_package` in your project to let `cmake` find the library, and link against the `xpress::xpress` target.

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

## Computing gradients

So far, we have provided the variables with respect to which we want to derive an expression.
You can also simply derive with respect to all variables in an expression, i.e. compute its gradient.
The `gradient_of` function essentially forwards to `derivatives_of`, so you again
have the possibility to get an object containing the derivative expressions,
or, compute the derivative values directly (see above). Here's an example for the latter case:

```cpp <!-- {{xpress-grad-snippet}} -->
var a;
var b;
auto derivs = gradient_of(a*log(b), at(a = 1.0, b = 2.0));
std::println("de_da = {}", derivs[a]);
std::println("de_db = {}", derivs[b]);
```

You may want to distinguish variables and parameters of your expression. To this end,
you may use `let` besides `var`, which behaves the same with the exception that it is not
interpreted as an independent variable, and thus, `gradient_of` will not differentiate with respect to `let`s:

```cpp <!-- {{xpress-grad_let-snippet}} -->
var a;
var b;
let c;
auto derivs = gradient_of(a*log(b)*c);
std::println("de_da = {}", derivs.wrt(a).with(a = "a", b = "b", c = "c"));
std::println("de_db = {}", derivs.wrt(b).with(a = "a", b = "b", c = "c"));
// this would not compile:
// derivs.wrt(c);
```

## Compile-time computations

All interfaces of this library are `constexpr`, so as long as the variable values are known at compile-time,
and your expression does not use functions that are not `constexpr` (for instance, `std::log` is not `constexpr` until c++-26),
you can do all computations during compilation:


```cpp <!-- {{xpress-comptimeeval-snippet}} -->
constexpr var a;
constexpr var b;
constexpr auto de_da = derivative_of(a*a + a*b, wrt(a), at(a = 2, b = 3));
static_assert(de_da == 2*2 + 3);
```

To simplify providing compile-time constants, the library provides `val`:

```cpp <!-- {{xpress-val-snippet}} -->
constexpr var a;
constexpr auto de_da = derivative_of(val<42>*a*a, wrt(a), at(a = 3));
static_assert(de_da == 42*2*3);
```

Compile-time derivatives can be useful if, for instance, a parameter of your
application depends on the solution of a small nonlinear equation. The library
provides a basic Newton solver that you can use to find the root of an expression
(potentially at compile-time):

```cpp <!-- {{xpress-newton-snippet}} -->
// #include <xpress/solvers/newton.hpp>
using namespace xp::solvers;
constexpr var a;
constexpr auto expression = a*a - val<1>;
constexpr auto solver = newton{{
    .threshold = 1e-6,
    .max_iterations = 10
}};
// find_root_of yields an std::optional (extract the result via value())
constexpr auto result = solver.find_root_of(expression, starting_from(a = 3.0))
                              .value();
// the solver supports vector expressions (see below), and thus, the result
// type is a container with the solutions to all variables of the expression
static_assert(result[a] - 1.0 < 1e-6);
```

## Vectorial and tensorial expressions

The following code snippet shows one way to create a vectorial expression and evaluate it:

```cpp <!-- {{xpress-vecexpr-snippet}} -->
var a;
var b;
auto velocity = vector_expression::from(a + b, a*b);
auto v = value_of(velocity, at(a = 1, b = 2));
std::println("v[0] = {}", v[0]);
std::println("v[1] = {}", v[1]);
```

Deriving a vectorial expression, each derivative is again a vectorial expression:

```cpp <!-- {{xpress-vecexprderiv-snippet}} -->
var a;
var b;
auto velocity = vector_expression::from(a/b, a*b);
auto dv_da = derivative_of(velocity, wrt(a)); // this a vector expr containing (1/b, b)

// to extract an individual expression in the vector expression, you can use
// an `index_constant`. With the template variable `ic` you can easily create them.
std::println("dv_da[0] = {}", dv_da[ic<0>].with(a = "a", b = "b"));
std::println("dv_da[1] = {}", dv_da[ic<1>].with(a = "a", b = "b"));
```

And, again, you can also directly evaluate the expressions:

```cpp <!-- {{xpress-vecexprderiveval-snippet}} -->
var a;
var b;
auto velocity = vector_expression::from(a/b, a*b);
auto dv_da = derivative_of(velocity, wrt(a), at(a = 1.0, b =2.0));
std::println("dv_da[0] = {}", dv_da[0]);
std::println("dv_da[1] = {}", dv_da[1]);
```

A vectorial expression is actually a `tensorial_expression` with one dimension,
so all the above also works with an arbitrary number of dimensions. To conveniently
construct tensorial expressions, you can use the `tensor_expression_builder`:

```cpp <!-- {{xpress-tensorexpr-snippet}} -->
var a;
var b;
// `at` is a convenience function to create `md_index` instances. You
// can also use the `md_ic` template variable, but here `at` reads better.
auto tensor = tensor_expression_builder{shape<2, 2>}
                .with(a*b, at<0, 0>())
                .with(a+b, at<0, 1>())
                .with(a/b, at<1, 0>())
                .with(a-b, at<1, 1>())
                .build();
// let's just print the expression at [0, 0]:
std::println("tensor[0, 0] = {}", tensor[md_ic<0, 0>].with(a = "a", b = "b"));
```

Evaluation of a tensorial expression yields a `xp::linalg::tensor`:

```cpp <!-- {{xpress-tensorexpreval-snippet}} -->
var a;
var b;
auto expr = tensor_expression_builder{shape<2, 2>}
                .with(a*b, at<0, 0>())
                .with(a+b, at<0, 1>())
                .with(a/b, at<1, 0>())
                .with(a-b, at<1, 1>())
                .build();
auto value = value_of(expr, at(a = 1.0, b = 2.0));
std::println("value[0, 0] = {}", value[0, 0]);
std::println("value[0, 1] = {}", value[0, 1]);
std::println("value[1, 0] = {}", value[1, 0]);
std::println("value[1, 1] = {}", value[1, 1]);
```

As for vectors, if you derive a tensor expression, you receive a tensor expression with the same shape.
Let's directly evaluate the derivative expression and get the resulting tensor:

```cpp <!-- {{xpress-tensorexprderiv-snippet}} -->
var a;
var b;
auto T = tensor_expression_builder{shape<2, 2>}
                .with(a*b, at<0, 0>())
                .with(a+b, at<0, 1>())
                .with(a/b, at<1, 0>())
                .with(a-b, at<1, 1>())
                .build();
auto dT_da = derivative_of(T, wrt(a), at(a = 1.0, b = 2.0));
std::println("dT_da[0, 0] = {}", dT_da[0, 0]);
std::println("dT_da[0, 1] = {}", dT_da[0, 1]);
std::println("dT_da[1, 0] = {}", dT_da[1, 0]);
std::println("dT_da[1, 1] = {}", dT_da[1, 1]);
```

## Vectors and tensors

Besides tensorial expressions, you can also define tensors or vectors, which are leaf symbols of your
expression (which contain sub-symbols). For instance, let's define a tensor and a vector symbol and
compute the matrix-vector product:

```cpp <!-- {{xpress-vecandtens-snippet}} -->
tensor T{shape<2, 2>};
vector<2> v;
auto Tv_expression = mat_mul(T, v);
// We have to bind tensors and vectors with the right shape
auto Tv = value_of(Tv_expression, at(
    T = xp::linalg::tensor{shape<2, 2>, 1, 2, 3, 4},
    v = std::array{1, 1}
));
// Tv is a vector with two entries
std::println("Tv[0] = {}", Tv[0]);
std::println("Tv[1] = {}", Tv[1]);
```

We can derive such expressions with respect to tensors/vectors, or, with respect to individual (sub-)symbols
of them. For instance, let's derive the above expression with respect to the tensor entry at (0, 0):

```cpp <!-- {{xpress-vecandtensderiv-snippet}} -->
tensor T{shape<2, 2>};
vector<2> v;
auto Tv_expression = mat_mul(T, v);
auto dTv_dT00 = derivative_of(Tv_expression, wrt(T[md_ic<0, 0>]), at(
    T = xp::linalg::tensor{shape<2, 2>, 1, 1, 1, 1},
    v = std::array{2, 3}
));
std::println("dTv_dT00[0] = {} (should be 2)", dTv_dT00[0]);
std::println("dTv_dT00[1] = {} (should be 0)", dTv_dT00[1]);
```

Finally, the following example computes the derivative of a scalar expression with respect to a tensor. The derivative
is again a tensor containing the derivatives with respect to the individual entries.

```cpp <!-- {{xpress-tensderiv-snippet}} -->
tensor T{shape<2, 2>};
auto scalar_product = T*T;
auto dsp_dT = derivative_of(scalar_product, wrt(T), at(T = linalg::tensor{shape<2, 2>, 1.0, 2.0, 3.0, 4.0}));
// should print dsp_dt = [2, 4, 6, 8]
std::println("dsp_dt = [{}, {}, {}, {}]", dsp_dT[0, 0], dsp_dT[0, 1], dsp_dT[1, 0], dsp_dT[1, 1]);
```
