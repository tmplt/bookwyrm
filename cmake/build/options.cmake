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
