cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(bookwyrm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${PROJECT_SOURCE_DIR}/cmake
  ${PROJECT_SOURCE_DIR}/cmake/modules)

include(utils)
include(build/core)
include(build/options)
#include(build/targets)
include(build/summary)

add_subdirectory(${PROJECT_SOURCE_DIR}/src)

#if(BUILD_TESTS)
#  add_subdirectory(${PROJECT_SOURCE_DIR}/tests ${PROJECT_BINARY_DIR}/tests)
#else()
#  add_subdirectory(${PROJECT_SOURCE_DIR}/tests ${PROJECT_BINARY_DIR}/tests EXCLUDE_FROM_ALL)
#endif()
