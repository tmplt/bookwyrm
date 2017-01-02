#
# Create executable
#

file(GLOB_RECURSE SOURCES RELATIVE ${PROJECT_SOURCE_DIR}/src *.c[p]*)

# Locate dependencies

#set(THREADS_PREFER_PTHREAD_FLAG ON)
# Required dependenc
#find_package(Threads REQUIRED)
#set(APP_LIBRARIES ${APP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# Required dependency: libcurl {{{

find_package(CURL REQUIRED)
set(APP_LIBRARIES ${APP_LIBRARIES} ${CURL_LIBRARY})
set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${CURL_INCLUDE_DIR})

# }}}

set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include)
set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

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

#target_link_libraries(${PROJECT_NAME} Threads::Threads)

# }}}
# Export target details {{{

set(APP_BINARY ${PROJECT_SOURCE_DIR}/bin/${PROJECT_NAME} PARENT_SCOPE)
set(APP_LIBRARIES ${APP_LIBRARIES} PARENT_SCOPE)
set(APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} PARENT_SCOPE)

# }}}

execute_process(COMMAND git describe --tags --dirty=-git
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  OUTPUT_VARIABLE PROG_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

configure_file(
  ${PROJECT_SOURCE_DIR}/include/config.hpp.cmake
  ${CMAKE_SOURCE_DIR}/include/config.hpp
  ESCAPE_QUOTES @ONLY)
