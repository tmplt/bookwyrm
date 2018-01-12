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

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(bookwyrm VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${PROJECT_SOURCE_DIR}/cmake)

include(utils)
include(build/core)
include(build/options)
include(build/summary)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/fmt)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/fuzzywuzzy)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pybind11)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/termbox)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    # Relocation R_X86_64_PC32 against symbols declared in the following libraries
    # cannot be used when making a shared object; compilation must be done with -fPIC
    set_target_properties(fmt PROPERTIES COMPILE_FLAGS "-fPIC")
    set_target_properties(fuzzywuzzy PROPERTIES COMPILE_FLAGS "-fPIC")
    set_target_properties(${PROJECT_NAME}-core PROPERTIES COMPILE_FLAGS "-fPIC")
endif()
