pybind11_add_module(pybookwyrm python.cpp)

target_include_directories(pybookwyrm
    PRIVATE ${PROJECT_SOURCE_DIR}/lib/fmt
    PUBLIC  ${PROJECT_SOURCE_DIR}/include/core)

target_link_libraries(pybookwyrm PRIVATE ${PROJECT_NAME}-core)
