/*
 * Copyright (C) 2017 Tmplt <tmplt@dragons.rocks>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <iostream>  // std::cout, std::endl

#include "version.hpp"

#define PROG_NAME "@PROJECT_NAME@"
#cmakedefine PROG_VERSION "@PROG_VERSION@"
#ifndef PROG_VERSION
    #define PROG_VERSION GIT_TAG
#endif

auto print_build_info = []() {
    std::cout << PROG_NAME << " " << PROG_VERSION << '\n'
              << "Copyright (C) 2017 Tmplt <tmplt@dragons.rocks>.\n"
              << PROG_NAME << " is licensed under the GPLv3+ license, "
              << "see <https://github.com/Tmplt/bookwyrm/blob/cpp/master/LICENSE>.\n\n"
              << "Written by Tmplt and others, see <https://github.com/Tmplt/bookwyrm/blob/cpp/master/AUTHORS.md>."
              << std::endl;
};

// vim: ft=cpp
