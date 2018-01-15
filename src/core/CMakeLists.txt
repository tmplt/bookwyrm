# Required external dependencies:
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(${PROJECT_NAME}-core STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plugin_handler.cpp)

target_include_directories(${PROJECT_NAME}-core
    PUBLIC  ${PROJECT_SOURCE_DIR}/include/core
    PUBLIC  ${PROJECT_SOURCE_DIR}/lib/fmt
    PRIVATE ${PROJECT_SOURCE_DIR}/lib/fuzzywuzzy/include
    PRIVATE ${PROJECT_SOURCE_DIR}/lib/pybind11/include)

target_link_libraries(${PROJECT_NAME}-core
    Threads::Threads
    fmt
    fuzzywuzzy
    pybind11::embed
    stdc++fs)

add_subdirectory(bindings)
