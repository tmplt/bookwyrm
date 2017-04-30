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

project(bookwyrm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${PROJECT_SOURCE_DIR}/cmake
  ${PROJECT_SOURCE_DIR}/cmake/modules)

include(utils)
include(build/core)
include(build/options)
include(build/summary)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib/fmt)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/fuzzywuzzy)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/pybind11)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
