`xpress` is a header-only C++ template library that let's you write strongly-typed mathematical expressions and,
for instance, derive them with respect to one or more variables at compile-time. To use the library, you may add
this repository as git submodule to yours, or, if your project uses `cmake`, you may use `FetchContent` to pull
it upon configuration. Alernatively, you can install the library locally on your system. See the documentation below
for examples on the latter two approaches.

## Prerequisites

- `cmake` (> 3.18)
- Compiler with support for `C++-23`

## Quick start

The following is a minimal example application that defines a simple expression, evaluates it and prints it to the terminal.

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
    auto expr = a + b;

    // evaluating the expression for specific values bound to the variables
    auto value = value_of(expr, at(a = 1, b = 2));
    std::println("v = {}", value);

    // invoking `with` on an expression, we can also bind values to the
    // symbols and create a printable (or streamable) `bound_expression`
    std::println("{}", expr.with(a = "a", b = "b")); // prints "a + b"
    std::println("{}", expr.with(a = 1, b = 2));     // prints "1 + 2"

    return 0;
}
```

The following is an exemplary `CMakeLists.txt` file that makes `xpress` automatically available upon configuration
and defines a single executable linked against the library.

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

With the two files above present in the same directory, you can compile and run this example with the following
sequence of commands:

```bash <!-- {{xpress-minimal-bash}} -->
cmake -B build
cmake --build build
cd build && ./my_app
```

Note that this assumes that your default compilers are recent enough. If not, you have to pass the
`cmake` flags `-DCMAKE_C_COMPILER` and `-DCMAKE_CXX_COMPILER` to have `cmake` use suitable compilers.
An alternative way of using the library is to install it on your system, for instance, with the following sequence
of commands:

```bash <!-- {{xpress-install-instructions}} -->
git clone --recursive https://github.com/dglaeser/xpress.git
cd xpress
cmake -DCMAKE_INSTALL_PREFIX=$(pwd)/xpress-install -DXPRESS_BUILD_TESTS=OFF -B build
cmake --build build
cmake --install build
```

Omit the `-DCMAKE_INSTALL_PREFIX` flag for a system-wide installation. After installation, you can use `cmake`'s
`find_package` command in your project to let `cmake` find the library, and link against the `xpress::xpress` target.

## Evaluating expressions

In order to evaluate an expression, we need to bind values to its leaf symbols. All symbols and expression types allow
to bind values to them via `operator=`, and we may use the `at` function to combine the results into a single data structure.
Finally, `value_of` evaluates the expression at the provided values.

```cpp <!-- {{xpress-eval-snippet}} -->
var a;
var b;
auto expr = a*a + b;
std::println("expr = {}", value_of(expr, at(a = 2.0, b = 3.0)));
```

You can also bind values to (sub-)expressions, which can speed up the computations if a sub-expression occurs multiple
times:

```cpp <!-- {{xpress-subexpr-snippet}} -->
var a;
var b;
auto arg = a*a + b;
auto expr = log(arg) + arg*arg;
auto arg_value = value_of(arg, at(a = 2.0, b = 3.0));
std::println("expr = {}", value_of(expr, with(arg = arg_value)));
```

Note that we have used `with` here, which is the same as `at`, but the different names allow you to distinguish binding
values to leaf symbols or (sub-)expressions (you can also concatenate multiple invocations, e.g. `auto bindings = at(...) & with(...)`).
Note also that since each `var` and each expression are unique types, compilation will fail if you don't provide all the
values required for an evaluation. By providing a value for `arg`, provision of values for the leaf symbols `a` and `b`
becomes obsolete in this example.

You can also create a callable from an expression by constructing an `evaluator` from it:

```cpp <!-- {{xpress-exprevaluator-snippet}} -->
var a;
var b;
evaluator f = a*log(b);
std::println("f(a=2, b=3) = {}", f(a = 2.0, b = 3.0));
```

### Available operators

To enable an operator (e.g. `*`, or `log`) for expressions, a small set of traits has to be implemented (depending on the
features you want to have available for it). All currently available operators can be found in the directory
[src/xpress/operators](src/xpress/operators). These implementations can serve as a blueprint for adding new operators.
If you find yourself implementing a generally useful operator, consider contributing it back to this project
(see [contribution guidelines](#contribution-guidelines)).

## Deriving expressions

Using `derivative_of`, you receive a new expression that is the derivative of the provided one with respect to the provided variables.

```cpp <!-- {{xpress-deriv-snippet}} -->
var a;
var b;
auto de_da = derivative_of(a*log(b), wrt(a));
std::println("de_da = {}", de_da.with(a = "a", b = "b"));
```

To directly compute the value of the derivative, you may provide values for the symbols of the expression:

```cpp <!-- {{xpress-deriveval-snippet}} -->
var a;
var b;
auto de_da = derivative_of(a*log(b), wrt(a), at(a = 42.0, b = 1.0));
std::println("de_da = {}", de_da);
```

As a side note, since `a` does not appear in the above derivative anymore, we would not have to provide a value for it.
However, usually you don't know the expression of the derivative in advance, and thus, providing values for all symbols should
be the default.

To get the derivatives with respect to multiple variables at once, you may use the `derivatives_of` function and access the
derivative expressions individually on the resulting data structure:

```cpp <!-- {{xpress-derivs-snippet}} -->
var a;
var b;
auto deriv_expressions = derivatives_of(a*log(b), wrt(a, b));
std::println("de_da = {}", deriv_expressions[a].with(a = "a", b = "b"));
std::println("de_db = {}", deriv_expressions[b].with(a = "a", b = "b"));
```

Again, you can also directly compute the values of all derivatives. The returned object also allows you to extract the
derivatives with respect to an individual variable by indexing into it with that variable.

```cpp <!-- {{xpress-derivseval-snippet}} -->
var a;
var b;
auto deriv_values = derivatives_of(a*log(b), wrt(a, b), at(a = 1.0, b = 2.0));
std::println("de_da = {}", deriv_values[a]);
std::println("de_db = {}", deriv_values[b]);
```

## Computing gradients

So far, we have provided the variables with respect to which we want to derive an expression.
You can also simply derive an expression with respect to all its variables, i.e. compute its gradient.
The `gradient_of` function is available in the same two variants as the `derivatives_of` function (see above),
with the only difference that you don't pass in the variables with respect to which to differentiate:

```cpp <!-- {{xpress-grad-snippet}} -->
var a;
var b;
auto derivs = gradient_of(a*log(b), at(a = 1.0, b = 2.0));
std::println("de_da = {}", derivs[a]);
std::println("de_db = {}", derivs[b]);
```

You may want to distinguish variables and parameters of your expression. To this end, you can use `let` besides `var`, which behaves
the same with the exception that it is not interpreted as an independent variable, and thus, `gradient_of` will not differentiate
with respect to `let`s:

```cpp <!-- {{xpress-grad_let-snippet}} -->
var a;
var b;
let c;
auto derivs = gradient_of(a*log(b)*c);
std::println("de_da = {}", derivs[a].with(a = "a", b = "b", c = "c"));
std::println("de_db = {}", derivs[b].with(a = "a", b = "b", c = "c"));
// this would not compile:
// derivs[c];
```

The data structures storing derivative expressions or values also allow you to iterate over all contained derivatives generically:

```cpp <!-- {{xpress-gradvisit-snippet}} -->
var a;
var b;
auto deriv_exprs = gradient_of(a*log(b));
visit(deriv_exprs, [&] (const auto& symbol, const auto& deriv_expr) {
    std::println("de_d{} = {}", (symbol == a ? "a" : "b"), deriv_expr.with(a = "a", b = "b"));
});
auto deriv_values = gradient_of(a*log(b), at(a = 1.0, b = 2.0));
visit(deriv_values, [&] (const auto& symbol, const auto& value) {
    std::println("de_d{} = {}", (symbol == a ? "a" : "b"), value);
});
```


## Constraining symbols on value types

By default, any type can be bound to a variable. If you want to constrain this, you may provide
an allowed data type as template argument:

```cpp <!-- {{xpress-dtype-snippet}} -->
var any;
var<int> only_int;
var<double> only_double;
var<dtype::real> any_floating_point;
var<dtype::integral> any_int;

any = "a";              // OK
only_double = 1.0;      // OK
only_double = 1;        // OK, int can be safely cast to double
only_int = 1;           // OK
// only_int = 1.0;      // compiler error, double cannot be safely cast to int
any_floating_point = 1; // OK, int to float is fine
any_int = 1;            // OK
// any_int = 1.0;       // compiler error, float to int is not possible
```


## Compile-time computations

All interfaces of this library are `constexpr`, so as long as the variable values are known at compile-time,
and your expression does not use functions that are not `constexpr` (for instance, `log` - which uses `std::log` under the hood - is not `constexpr` until c++-26),
you can do all computations during compilation:


```cpp <!-- {{xpress-comptimeeval-snippet}} -->
constexpr var a;
constexpr var b;
constexpr auto de_da = derivative_of(a*a + a*b, wrt(a), at(a = 2, b = 3));
static_assert(de_da == 2*2 + 3);
```

To simplify providing compile-time constant values, the library provides `val`:

```cpp <!-- {{xpress-val-snippet}} -->
constexpr var a;
constexpr auto de_da = derivative_of(val<42>*a*a, wrt(a), at(a = 3));
static_assert(de_da == 42*2*3);
```

Compile-time derivatives can be useful if, for instance, a parameter of your application depends on the solution of a small
nonlinear equation. The library provides a basic Newton solver that you can use to find the root of an expression
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
std::println("dv0_da = {}", dv_da[ic<0>].with(a = "a", b = "b"));
std::println("dv1_da = {}", dv_da[ic<1>].with(a = "a", b = "b"));
```

And, again, you can also directly evaluate the expressions:

```cpp <!-- {{xpress-vecexprderiveval-snippet}} -->
var a;
var b;
auto velocity = vector_expression::from(a/b, a*b);
auto dv_da = derivative_of(velocity, wrt(a), at(a = 1.0, b =2.0));
std::println("dv0_da = {}", dv_da[0]);
std::println("dv1_da = {}", dv_da[1]);
```

A vectorial expression is actually a `tensorial_expression` with one dimension,
so all the above also works with an arbitrary number of dimensions. To conveniently
construct tensorial expressions, you can use the `tensor_expression_builder`:

```cpp <!-- {{xpress-tensorexpr-snippet}} -->
var a;
var b;
// `at<size_t...>` is a convenience function to create `md_index` instances. You
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

Evaluation of a tensorial expression yields a `xp::linalg::tensor` (the vectorial examples above did so, too, btw):

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
std::println("dT00_da = {}", dT_da[0, 0]);
std::println("dT01_da = {}", dT_da[0, 1]);
std::println("dT10_da = {}", dT_da[1, 0]);
std::println("dT11_da = {}", dT_da[1, 1]);
```

## Vectors and tensors

Besides tensorial expressions, you can also define tensors or vectors, which act as leaf symbols of expressions
(which again contain sub-symbols). For instance, let's define a tensor and a vector and compute the matrix-vector product:

```cpp <!-- {{xpress-vecandtens-snippet}} -->
tensor T{shape<2, 2>};
vector<2> v;
auto Tv_expression = mat_mul(T, v);
// We have to bind tensors and vectors with the right shape
auto Tv = value_of(Tv_expression, at(
    T = xp::linalg::tensor{shape<2, 2>, 1, 2, 3, 4},
    v = std::array{1, 1}  // could also be a linalg::tensor{shape<2>,...}
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
std::println("dTv0_dT00 = {} (should be 2)", dTv_dT00[0]);
std::println("dTv1_dT00 = {} (should be 0)", dTv_dT00[1]);
```

Finally, the following example computes the derivative of a scalar expression with respect to a tensor. The derivative
is again a tensor containing the derivatives of the expression with respect to the individual tensor entries.

```cpp <!-- {{xpress-tensderiv-snippet}} -->
tensor T{shape<2, 2>};
auto scalar_product = T*T;
auto dsp_dT = derivative_of(scalar_product, wrt(T), at(T = linalg::tensor{shape<2, 2>, 1.0, 2.0, 3.0, 4.0}));
// should print dsp_dt = [2  4; 6 8]
std::println("dsp_dT = [{} {}; {} {}]", dsp_dT[0, 0], dsp_dT[0, 1], dsp_dT[1, 0], dsp_dT[1, 1]);
```

### Custom vector/tensor types

In the examples so far, we have used the `xp::linalg::tensor` class to represent tensorial/vectorial values. If you are working
with custom data structures, you can make them compatible with the library by implementing a few traits classes.
See the `tensorial` concept in [tensor.hpp](src/xpress/tensor.hpp) for details. If your tensor data structures
are something like nested arrays, that is, expose an `operator[]` to obtain a "sub-tensor", then your data structures are likely
to be compatible out-of-the-box (because then the default implementations of the traits would be available for your type).
Tensorial results from evaluating expressions are always represented by `xp::linalg::tensor`s, however, if your type is compatible
(via the above-mentioned traits), you can easily export results back to your own data structures:

```cpp <!-- {{xpress-tensorexport-snippet}} -->
using my_tensor_type = std::array<std::array<int, 3>, 2>;
my_tensor_type my_tensor;
linalg::tensor{shape<2, 3>, 1, 2, 3, 4, 5, 6}.export_to(my_tensor);
```

Note also that the library provides a mechanism that allows you to hook into custom operator implementation on your tensor types,
by specializing the respective traits in the namespace `xp::operators::traits` (see any of the available operators in [src/xpress/operators](src/xpress/operators)).


## Contribution guidelines

Contributions are highly welcome! For bug reports, please file an [issue](https://github.com/dglaeser/xpress/issues).
If you want to contribute with features, improvements or bug fixes please fork this project and open a pull request into the main branch of this repository.


## Acknowledgements

An initial implementation of this library has been largely refactored after inspiration from Vincent Reverdy's [talk at CppCon2023](https://www.youtube.com/watch?v=lPfA4SFojao). Moreover, some of the API design is inspired by the [autodiff library](https://github.com/autodiff/autodiff).
