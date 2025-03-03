# SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.a.glaeser@gmail.com>
# SPDX-License-Identifier: MIT

include(FetchContent)
FetchContent_Declare(
  ut
  GIT_REPOSITORY https://github.com/boost-ext/ut
  GIT_TAG v1.1.9
)
FetchContent_MakeAvailable(ut)

function (xpress_add_test NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} PRIVATE xpress::xpress Boost::ut)
    add_test(NAME ${NAME} COMMAND ./${NAME})
endfunction ()

xpress_add_test(test_utils test_utils.cpp)
xpress_add_test(test_linalg test_linalg.cpp)
xpress_add_test(test_dtype test_dtype.cpp)
xpress_add_test(test_symbols test_symbols.cpp)
xpress_add_test(test_bindings test_bindings.cpp)
xpress_add_test(test_operators test_operators.cpp)
xpress_add_test(test_expression_stream test_expression_stream.cpp)
xpress_add_test(test_tensor test_tensor.cpp)
xpress_add_test(test_solvers test_solvers.cpp)
