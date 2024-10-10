include(FetchContent)
FetchContent_Declare(
  ut
  GIT_REPOSITORY https://github.com/boost-ext/ut
  GIT_TAG v1.1.9
)
FetchContent_MakeAvailable(ut)

find_package(adac)
function (ad_add_test NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} PRIVATE adac::adac Boost::ut)
    add_test(NAME ${NAME} COMMAND ./${NAME})
endfunction ()

ad_add_test(test_utils test_utils.cpp)
ad_add_test(test_linalg test_linalg.cpp)
ad_add_test(test_dtype test_dtype.cpp)
ad_add_test(test_symbols test_symbols.cpp)
ad_add_test(test_bindings test_bindings.cpp)
ad_add_test(test_operators test_operators.cpp)
ad_add_test(test_expression_stream test_expression_stream.cpp)
ad_add_test(test_tensor test_tensor.cpp)
