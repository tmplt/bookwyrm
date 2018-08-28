pybind11_add_module(pybookwyrm python.cpp ../../string.cpp)

target_include_directories(pybookwyrm
    PRIVATE ${PROJECT_SOURCE_DIR}/lib/fmt
    PUBLIC  ${PROJECT_SOURCE_DIR}/include/core)

target_link_libraries(pybookwyrm PRIVATE ${PROJECT_NAME}-core)

# Determine install directory
# TODO: replace this with FindPython3 when it's released
find_package(PythonInterp 3.0 REQUIRED)
execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c
    "from distutils.sysconfig import get_python_lib; print(get_python_lib(plat_specific=True, prefix=''))"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE PYTHON3_INSTALL_DIR
)
message_colored(STATUS "Python 3 site-packages directory: ${CMAKE_INSTALL_PREFIX}/${PYTHON3_INSTALL_DIR}" 33)

install(TARGETS pybookwyrm LIBRARY DESTINATION ${PYTHON3_INSTALL_DIR})
