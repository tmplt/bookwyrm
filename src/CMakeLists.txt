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

#
# Create executable
#

file(GLOB_RECURSE SOURCES RELATIVE ${PROJECT_SOURCE_DIR}/src *.c[p]*)

# Required dependencies
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED QUIET)

set(APP_LIBRARIES ${APP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# Include-order matters; if there is a header clash, the project-local headers are used.
set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include)
set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

# Required dependency: spdlog {{{

set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/lib/spdlog/include)

# }}}
# Required dependency: fmtlib {{{

set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/lib/fmt/)

# }}}
# Required dependency: fmtlib {{{

set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/lib/fuzzywuzzy/include)

# }}}
# Required dependency: pybind11 {{{

set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/lib/pybind11/include)

# }}}

if(ENABLE_IRC)
  add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libircppclient ${PROJECT_SOURCE_DIR}/lib/libircppclient)
  set(APP_LIBRARIES ${APP_LIBRARIES} ${LIBIRCPPCLIENT_LIBRARIES})
  set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${LIBIRCPPCLIENT_INCLUDE_DIRS})
  #else()
  #  list(REMOVE_ITEM SOURCES modules/??? utils/???)
endif()

# Create executable target {{{

make_executable(${PROJECT_NAME}
    SOURCES
      ${SOURCES}
    INCLUDE_DIRS
      ${APP_INCLUDE_DIRS}
    RAW_DEPENDS
      ${APP_LIBRARIES})

target_link_libraries(${PROJECT_NAME} Threads::Threads fmt fuzzywuzzy)

# }}}
# Export target details {{{

set(APP_BINARY ${PROJECT_SOURCE_DIR}/bin/${PROJECT_NAME} PARENT_SCOPE)
set(APP_LIBRARIES ${APP_LIBRARIES} PARENT_SCOPE)
set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} PARENT_SCOPE)

# }}}

# HELP WANTED:
# and the next to last occurance of '-' with '+'
# to make it comatible with semantic versioning.
execute_process(COMMAND git describe --tags --dirty=-git
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  OUTPUT_VARIABLE PROG_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

configure_file(
  ${PROJECT_SOURCE_DIR}/include/version.hpp.cmake
  ${CMAKE_SOURCE_DIR}/include/version.hpp
  ESCAPE_QUOTES @ONLY)
