#
# Build options
#

# Define build options {{{

option(CXXLIB_CLANG       "Link against libc++"        OFF)
option(CXXLIB_GCC         "Link against stdlibc++"     OFF)

option(BUILD_TESTS        "Build testsuite"            OFF)
option(DEBUG_LOGGER       "Enable extra debug logging" OFF)
option(VERBOSE_TRACELOG   "Enable verbose trace logs"  OFF)
option(DEBUG_HINTS        "Enable hints rendering"     OFF)

# }}}
