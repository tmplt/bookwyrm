# Enables some functionality
cmake_minimum_required(VERSION 3.0)
project(bookwyrm VERSION 0.1.0)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=1")

# Get the latest abbreviated commit hash of the working branch
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.\
${PROJECT_VERSION_PATCH}")

add_definitions(-std=c++1y)

# Linux Clang + AppleClang
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_SHARED_LINKER_FLAGS "-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-unused-parameters")

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_SHARED_LINKER_FLAGS "-Wall")

endif()

# For pthreads.
#set(THREADS_PREFER_PTHEAD_FLAG ON)

#find_package(Boost COMPONENTS system REQUIRED)
#find_package(Threads REQUIRED)
#find_package(OpenSSL REQUIRED)

#set(LINK_LIBS libircppclient ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES})

# Include CMakeLists.txt from src/ and tests/
add_subdirectory(src)

string(CONCAT PROJECT_NAME_VERSION ${PROJECT_NAME}\ v${VERSION_STRING}+${GIT_COMMIT_HASH})

message("
        -----------------------------
        ${PROJECT_NAME_VERSION}
        -----------------------------

        Compiler      | ${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}
        Compile flags | ${CMAKE_SHARED_LINKER_FLAGS}
        Build dir     | ${CMAKE_BINARY_DIR}
")

