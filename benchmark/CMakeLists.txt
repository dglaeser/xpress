include(FetchContent)

set(AUTODIFF_BUILD_TESTS OFF CACHE INTERNAL "")
set(AUTODIFF_BUILD_PYTHON OFF CACHE INTERNAL "")
set(AUTODIFF_BUILD_EXAMPLES OFF CACHE INTERNAL "")
set(AUTODIFF_BUILD_DOCS OFF CACHE INTERNAL "")
FetchContent_Declare(
  autodiff
  GIT_REPOSITORY https://github.com/autodiff/autodiff
  GIT_TAG v1.1.2
)
FetchContent_MakeAvailable(autodiff)

find_package(xpress)
function (xpress_add_benchmark NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} PRIVATE xpress::xpress autodiff::autodiff)
    add_test(NAME ${NAME} COMMAND ./${NAME})
endfunction ()

xpress_add_benchmark(expression_evaluation expression_evaluation.cpp)
xpress_add_benchmark(expression_differentiation expression_differentiation.cpp)

xpress_add_benchmark(expression_evaluation_autodiff_forward expression_evaluation.cpp)
xpress_add_benchmark(expression_evaluation_autodiff_backward expression_evaluation.cpp)
target_compile_definitions(expression_evaluation_autodiff_forward PRIVATE USE_AUTODIFF=1 USE_AUTODIFF_BACKWARD=0)
target_compile_definitions(expression_evaluation_autodiff_backward PRIVATE USE_AUTODIFF=1 USE_AUTODIFF_BACKWARD=1)

xpress_add_benchmark(expression_differentiation_autodiff_forward expression_differentiation.cpp)
xpress_add_benchmark(expression_differentiation_autodiff_backward expression_differentiation.cpp)
target_compile_definitions(expression_differentiation_autodiff_forward PRIVATE USE_AUTODIFF=1 USE_AUTODIFF_BACKWARD=0)
target_compile_definitions(expression_differentiation_autodiff_backward PRIVATE USE_AUTODIFF=1 USE_AUTODIFF_BACKWARD=1)
