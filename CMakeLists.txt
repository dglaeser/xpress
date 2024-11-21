cmake_minimum_required(VERSION 3.18)

project(xpress)

set(XPRESS_NAMESPACE xpress)
option(XPRESS_BUILD_TESTS "Control if tests should be built" ON)
# option(XPRESS_BUILD_EXAMPLES "Control if examples should be built" OFF)
option(XPRESS_BUILD_BENCHMARK "Control if benchmarks should be built" OFF)

include(GNUInstallDirs)
add_library(xpress INTERFACE)
target_compile_features(xpress INTERFACE cxx_std_23)
target_include_directories(xpress
    INTERFACE $<BUILD_INTERFACE:${xpress_SOURCE_DIR}/src>
              $<BUILD_INTERFACE:${xpress_SOURCE_DIR}/deps/cpputils/src>
              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
              $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/xpress/deps/>
)

# Alias to be used in test suite or when included as sub-project
add_library(xpress::xpress ALIAS xpress)

include(GNUInstallDirs)
set(XPRESS_INSTALL_CMAKE_DATA_DIR "${CMAKE_INSTALL_DATAROOTDIR}/xpress/cmake")
install(
    DIRECTORY src/xpress
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
    DIRECTORY deps/cpputils/src/cpputils
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xpress/deps
)
install(
    TARGETS xpress
    EXPORT xpress_Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
    EXPORT xpress_Targets
    FILE xpressTargets.cmake
    NAMESPACE ${XPRESS_NAMESPACE}::
    DESTINATION ${XPRESS_INSTALL_CMAKE_DATA_DIR}
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/xpress-config.cmake.in"
    "${PROJECT_BINARY_DIR}/cmake/pkg/xpressConfig.cmake"
    PATH_VARS
        CMAKE_INSTALL_INCLUDEDIR
        XPRESS_INSTALL_CMAKE_DATA_DIR
    INSTALL_DESTINATION
        ${XPRESS_INSTALL_CMAKE_DATA_DIR}
)
install(
    FILES
        "${PROJECT_BINARY_DIR}/cmake/pkg/xpressConfig.cmake"
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
