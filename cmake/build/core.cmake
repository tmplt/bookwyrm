#
# Core setup
#
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands used for custom targets
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  message_colored(STATUS "No build type specified; using Debug" 33)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE")
  set(BUILDING_DEBUG TRUE)
else()
  set(BUILDING_RELEASE TRUE)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wdouble-promotion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -O0 -g2")

# Check compiler
if(CMAKE_COMPILER_IS_GNUXX)
    message_colored(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER_ID} not supported (only gcc is supported currently; see <https://github.com/Tmplt/bookwyrm/issues/51>.)" 31)
else()
    message_colored(STATUS "Using ${CMAKE_CXX_COMPILER_ID} compiler." 33)
endif()

if(ENABLE_CCACHE)
  find_program(ccache_program ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${ccache_program})
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ${ccache_program})
endif()

# Install paths
if(NOT DEFINED CMAKE_INSTALL_BINDIR)
  set(CMAKE_INSTALL_BINDIR bin)
endif()
if(NOT DEFINED CMAKE_INSTALL_SBINDIR)
  set(CMAKE_INSTALL_SBINDIR sbin)
endif()
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR lib)
endif()
if(NOT DEFINED CMAKE_INSTALL_INCLUDEDIR)
  set(CMAKE_INSTALL_INCLUDEDIR include)
endif()
