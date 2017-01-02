#pragma once

#include <iostream>

#include "version.hpp"

#define PROG_NAME "@PROJECT_NAME@"
#cmakedefine PROG_VERSION "@PROG_VERSION@"
#ifndef PROG_VERSION
    #define PROG_VERSION GIT_TAG
#endif

auto print_build_info = []() {
    std::cout << PROG_NAME << " " << PROG_VERSION << '\n'
              << "Copyright (C) 2016-2017 Tmplt <tmplt@dragons.rocks>.\n"
              << PROG_NAME << " is licensed under the MIT license, "
              << "see <https://github.com/Tmplt/bookwyrm/blob/cpp/master/LICENSE.md>.\n\n"
              << "Written by Tmplt and others, see <https://github.com/Tmplt/bookwyrm/blob/cpp/master/AUTHORS.md>."
              << std::endl;
};

// vim: ft=cpp
