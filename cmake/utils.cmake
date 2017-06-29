#
# Collection of cmake utility functions
#

# message_colored : Outputs a colorized message {{{

function(message_colored message_level text color)
  string(ASCII 27 esc)
  message(${message_level} "${esc}[${color}m${text}${esc}[0m")
endfunction()

# }}}
# make_executable : Builds an executable target {{{

function(make_executable target_name)
  set(zero_value_args)
  set(one_value_args PACKAGE)
  set(multi_value_args SOURCES INCLUDE_DIRS PKG_DEPENDS CMAKE_DEPENDS TARGET_DEPENDS RAW_DEPENDS)

  cmake_parse_arguments(BIN
    "${zero_value_args}" "${one_value_args}"
    "${multi_value_args}" ${ARGN})

  # add defined INCLUDE_DIRS
  include_directories(${BIN_INCLUDE_DIRS})

  # add INCLUDE_DIRS for all external dependencies
  foreach(DEP ${BIN_TARGET_DEPENDS} ${BIN_PKG_DEPENDS} ${BIN_CMAKE_DEPENDS})
    string(TOUPPER ${DEP} DEP)
    include_directories(${${DEP}_INCLUDE_DIRS})
    include_directories(${${DEP}_INCLUDEDIR})
  endforeach()

  # create target
  add_executable(${target_name} ${BIN_SOURCES})

  # set the output file basename the same for static and shared
  set_target_properties(${target_name}
                        PROPERTIES OUTPUT_NAME ${target_name})

  # link libraries from pkg-config imports
  foreach(DEP ${BIN_PKG_DEPENDS})
    string(TOUPPER ${DEP} DEP)
    target_link_libraries(${target_name} ${${DEP}_LDFLAGS})
  endforeach()

  # link libraries from cmake imports
  foreach(DEP ${BIN_CMAKE_DEPENDS})
    string(TOUPPER ${DEP} DEP)
    target_link_libraries(${target_name} ${${DEP}_LIB}
                                             ${${DEP}_LIBRARY}
                                             ${${DEP}_LIBRARIES})
  endforeach()

  # link libraries that are build as part of this project
  target_link_libraries(${target_name} ${BIN_TARGET_DEPENDS} ${BIN_RAW_DEPENDS})

  # install targets
  install(TARGETS ${target_name}
          RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
          LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
          ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
          COMPONENT runtime)
endfunction()

# }}}
