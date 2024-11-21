cmake_minimum_required(VERSION 3.18)

if(NOT DEFINED PROJECT_NAME)
  set(XPRESS_SUBPROJECT OFF)
else()
  set(XPRESS_SUBPROJECT ON)
endif()

project(xpress)

set(XPRESS_NAMESPACE xpress)
option(XPRESS_BUILD_TESTS "Control if tests should be built" ON)
# option(XPRESS_BUILD_EXAMPLES "Control if examples should be built" OFF)
option(XPRESS_BUILD_BENCHMARK "Control if benchmarks should be built" OFF)

include(GNUInstallDirs)
add_library(${PROJECT_NAME} INTERFACE)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)
target_include_directories(${PROJECT_NAME}
    INTERFACE $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/src>
              $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/deps/cpputils/src>
              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/xpress/deps/>
)

# Alias to be used in test suite or when included as sub-project
add_library(${XPRESS_NAMESPACE}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (NOT XPRESS_SUBPROJECT)
    include(GNUInstallDirs)
    set(XPRESS_INSTALL_CMAKE_DATA_DIR "${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake")
    install(
        DIRECTORY src/${PROJECT_NAME}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    install(
        DIRECTORY deps/cpputils/src/cpputils
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/deps
    )
    install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(
        EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${XPRESS_NAMESPACE}::
        DESTINATION ${XPRESS_INSTALL_CMAKE_DATA_DIR}
    )

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
        "${PROJECT_BINARY_DIR}/cmake/pkg/${PROJECT_NAME}Config.cmake"
        PATH_VARS
            CMAKE_INSTALL_INCLUDEDIR
            XPRESS_INSTALL_CMAKE_DATA_DIR
        INSTALL_DESTINATION
            ${XPRESS_INSTALL_CMAKE_DATA_DIR}
    )
    install(
        FILES
            "${PROJECT_BINARY_DIR}/cmake/pkg/${PROJECT_NAME}Config.cmake"
        DESTINATION
            ${XPRESS_INSTALL_CMAKE_DATA_DIR}
    )

    if (XPRESS_BUILD_BENCHMARK)
        enable_testing()
        add_subdirectory(benchmark)
    endif ()

    if (XPRESS_BUILD_TESTS)
        enable_testing()
        add_subdirectory(test)
    endif ()

    # if (XPRESS_BUILD_EXAMPLES)
    #     enable_testing()
    #     add_subdirectory(examples)
    # endif ()
endif ()
