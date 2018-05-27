cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(bookwyrm VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${PROJECT_SOURCE_DIR}/cmake)

include(FindPythonInterp)
include(utils)
include(build/core)
include(build/options)
include(build/summary)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/fmt)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/fuzzywuzzy)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pybind11)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    # Relocation R_X86_64_PC32 against symbols declared in the following libraries
    # cannot be used when making a shared object; compilation must be done with -fPIC
    set_target_properties(fmt PROPERTIES COMPILE_FLAGS "-fPIC")
    set_target_properties(fuzzywuzzy PROPERTIES COMPILE_FLAGS "-fPIC")
    set_target_properties(${PROJECT_NAME}-core PROPERTIES COMPILE_FLAGS "-fPIC")
endif()
