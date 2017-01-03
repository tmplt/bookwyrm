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
# Build options
#

# Default value for: ENABLE_IRC {{{

find_package(libircppclient QUIET)
if(NOT DEFINED ENABLE_IRC AND NOT libircppclient)
    set(ENABLE_IRC OFF CACHE STRING "Support IRC sources")
endif()

# }}}

# Define build options {{{

option(CXXLIB_CLANG       "Link against libc++"        OFF)
option(CXXLIB_GCC         "Link against stdlibc++"     OFF)

option(BUILD_TESTS        "Build testsuite"            OFF)
option(DEBUG_LOGGER       "Enable extra debug logging" OFF)
option(VERBOSE_TRACELOG   "Enable verbose trace logs"  OFF)
option(DEBUG_HINTS        "Enable hints rendering"     OFF)

option(ENABLE_IRC         "Enable IRC sources"         OFF)

# }}}
