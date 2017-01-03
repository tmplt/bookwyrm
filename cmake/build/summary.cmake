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
# Output build summary
#

function(colored_option message_level text var color_on color_off)
  string(ASCII 27 esc)
  if(${var})
    message(${message_level} "${esc}[${color_on}m${text}${esc}[0m")
  else()
    message(${message_level} "${esc}[${color_off}m${text}${esc}[0m")
  endif()
endfunction()

message(STATUS "--------------------------")
if(CMAKE_BUILD_TYPE)
  message_colored(STATUS " Build type: ${CMAKE_BUILD_TYPE}" "32;1")
else()
  message_colored(STATUS " Build type: NONE" "33;1")
endif()

message(STATUS " Compiler C: ${CMAKE_C_COMPILER}")
message(STATUS " Compiler C++: ${CMAKE_CXX_COMPILER}")
message(STATUS " Compiler flags: ${CMAKE_CXX_FLAGS}")

string(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
if(CMAKE_BUILD_TYPE_LOWER STREQUAL "debug")
  message(STATUS "  debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
  if(NOT DEFINED ${DEBUG_LOGGER})
    set(DEBUG_LOGGER ON)
  endif()
  if(NOT DEFINED ${ENABLE_CCACHE})
    set(ENABLE_CCACHE ON)
  endif()
elseif(CMAKE_BUILD_TYPE_LOWER STREQUAL "release")
  message(STATUS "   release: ${CMAKE_CXX_FLAGS_RELEASE}")
elseif(CMAKE_BUILD_TYPE_LOWER STREQUAL "sanitize")
  message(STATUS "   sanitize: ${CMAKE_CXX_FLAGS_SANITIZE}")
elseif(CMAKE_BUILD_TYPE_LOWER STREQUAL "minsizerel")
  message(STATUS "   minsizerel: ${CMAKE_CXX_FLAGS_MINSIZEREL}")
elseif(CMAKE_BUILD_TYPE_LOWER STREQUAL "relwithdebinfo")
  message(STATUS "   relwithdebinfo: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif()

if(CMAKE_EXE_LINKER_FLAGS)
  message(STATUS " Linker flags: ${CMAKE_EXE_LINKER_FLAGS}")
endif()

if(CXXLIB_CLANG)
  message(STATUS " C++ library: libc++")
elseif(CXXLIB_GCC)
  message(STATUS " C++ library: libstdc++")
else()
  message(STATUS " C++ library: system default")
endif()

message(STATUS "--------------------------")
colored_option(STATUS " Build testsuite      ${BUILD_TESTS}" BUILD_TESTS "32;1" "37;2")
colored_option(STATUS " Debug logging        ${DEBUG_LOGGER}" DEBUG_LOGGER "32;1" "37;2")
colored_option(STATUS " Verbose tracing      ${VERBOSE_TRACELOG}" VERBOSE_TRACELOG "32;1" "37;2")
colored_option(STATUS " Draw debug hints     ${DEBUG_HINTS}" DEBUG_HINTS "32;1" "37;2")
colored_option(STATUS " Enable ccache        ${ENABLE_CCACHE}" ENABLE_CCACHE "32;1" "37;2")
message(STATUS "--------------------------")
colored_option(STATUS " Enable irc           ${ENABLE_IRC}" ENABLE_IRC "32;1" "37;2")
message(STATUS "--------------------------")
