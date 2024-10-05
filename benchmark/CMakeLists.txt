find_package(adac)
function (adac_add_benchmark NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME} PRIVATE adac::adac)
    add_test(NAME ${NAME} COMMAND ./${NAME})
endfunction ()

adac_add_benchmark(expression_evaluation expression_evaluation.cpp)
adac_add_benchmark(expression_differentiation expression_differentiation.cpp)
