cmake_minimum_required(VERSION 3.4.3 FATAL_ERROR)

find_program(ccache_program ccache)
if(ccache_program)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${ccache_program}")
endif()

project(bookwyrm VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${PROJECT_SOURCE_DIR}/cmake)

include(FindPythonInterp)
include(utils)
include(build/core)
include(build/options)
include(build/summary)

enable_testing()

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/fmt)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/fuzzywuzzy)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pybind11)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/tests)

file(GLOB_RECURSE check_cxx_source_files src/*.[ch]pp)
include(cmake/clang-cpp-checks.cmake)
