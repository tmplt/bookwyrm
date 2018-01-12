# Copyright (C) 2017 Tmplt <tmplt@dragons.rocks>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

find_package(CURL REQUIRED)

add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/utils.cpp
    ${PROJECT_SOURCE_DIR}/src/keys.cpp
    ${PROJECT_SOURCE_DIR}/src/logger.cpp
    ${PROJECT_SOURCE_DIR}/src/command_line.cpp
    ${PROJECT_SOURCE_DIR}/src/screen_butler.cpp
    ${PROJECT_SOURCE_DIR}/src/downloader.cpp
    ${PROJECT_SOURCE_DIR}/src/screens/base.cpp
    ${PROJECT_SOURCE_DIR}/src/screens/multiselect_menu.cpp
    ${PROJECT_SOURCE_DIR}/src/screens/item_details.cpp
    ${PROJECT_SOURCE_DIR}/src/screens/log.cpp)

target_include_directories(${PROJECT_NAME} BEFORE PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME}
    PRIVATE ${PROJECT_SOURCE_DIR}/lib/spdlog/include
    PRIVATE ${PROJECT_SOURCE_DIR}/lib/fmt
    PRIVATE ${PROJECT_SOURCE_DIR}/lib/termbox/src
    PRIVATE ${CURL_INCLUDE_DIRS})

# Some pre-compile tasks:
execute_process(COMMAND git describe --tags --dirty=-git
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  OUTPUT_VARIABLE PROG_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

configure_file(
  ${PROJECT_SOURCE_DIR}/include/version.hpp.cmake
  ${CMAKE_SOURCE_DIR}/include/version.hpp
  ESCAPE_QUOTES @ONLY)

target_link_libraries(${PROJECT_NAME}
    fmt
    stdc++fs
    termbox_lib_static
    ${CURL_LIBRARIES}
    ${PROJECT_NAME}-core)

add_subdirectory(core)
