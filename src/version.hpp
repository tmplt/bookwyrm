/*
 * A pre-generation header (or post, if this is config.hpp),
 * used by cmake to create an up-to-date version string.
 */

#pragma once

#include <iostream>

#include "version.hpp"

#define PROG_NAME "bookwyrm"
    #define PROG_VERSION "v0.4.0-96-gaa3805e-git"
#ifndef PROG_VERSION
    #define PROG_VERSION GIT_TAG
#endif

constexpr auto print_build_info = []() {
    std::cout << PROG_NAME << ' ' << PROG_VERSION << '\n'
              << "Copyright (C) 2016-2018 Tmplt <tmplt@dragons.rocks>\n"
              << PROG_NAME << " is licensed under the MIT license.\n\n"
              << "Written by Tmplt and others, see <https://github.com/Tmplt/bookwyrm/blob/master/AUTHORS.md>."
              << std::endl;
};

const auto build_info_short = '\"' + string(PROG_VERSION) + '\"';

// vim: ft=cpp
