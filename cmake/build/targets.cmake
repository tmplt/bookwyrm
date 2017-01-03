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
# Custom targets
#

# Target: userconfig {{{

configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/templates/userconfig.cmake.in
  ${PROJECT_BINARY_DIR}/userconfig.cmake
  IMMEDIATE @ONLY)

add_custom_target(userconfig COMMAND ${CMAKE_COMMAND}
  -P ${PROJECT_BINARY_DIR}/userconfig.cmake)

# }}}
# Target: uninstall {{{

configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/templates/uninstall.cmake.in
  ${PROJECT_BINARY_DIR}/cmake/uninstall.cmake
  IMMEDIATE @ONLY)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND}
  -P ${PROJECT_BINARY_DIR}/cmake/uninstall.cmake)

# }}}
# Target: codeformat (clang-format) {{{

add_custom_target(codeformat)
add_custom_command(TARGET codeformat COMMAND
  ${PROJECT_SOURCE_DIR}/common/clang-format.sh ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)

# }}}
# Target: codecheck (clang-tidy) {{{

add_custom_target(codecheck)
add_custom_command(TARGET codecheck COMMAND
  ${PROJECT_SOURCE_DIR}/common/clang-tidy.sh ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/src)

# }}}
# Target: codecheck-fix (clang-tidy + clang-format) {{{

add_custom_target(codecheck-fix)
add_custom_command(TARGET codecheck-fix COMMAND
  ${PROJECT_SOURCE_DIR}/common/clang-tidy.sh ${PROJECT_BINARY_DIR} -fix ${PROJECT_SOURCE_DIR}/src)

# }}}

# Target: memcheck (valgrind) {{{

add_custom_target(memcheck)
add_custom_command(TARGET memcheck COMMAND valgrind
  --leak-check=summary
  --suppressions=${PROJECT_SOURCE_DIR}/.valgrind-suppressions
  ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME} example --config=${PROJECT_SOURCE_DIR}/doc/config)

add_custom_target(memcheck-full)
add_custom_command(TARGET memcheck-full COMMAND valgrind
  --leak-check=full
  --track-origins=yes
  --track-fds=yes
  --suppressions=${PROJECT_SOURCE_DIR}/.valgrind-suppressions
  ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}/${PROJECT_NAME} example --config=${PROJECT_SOURCE_DIR}/doc/config)

# }}}
