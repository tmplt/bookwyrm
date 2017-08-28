/*
 * This file contains a bunch of utility functions,
 *
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

#include <unistd.h>
#include <algorithm>
#include <system_error>
#include <experimental/filesystem>

#include "common.hpp"

namespace fs = std::experimental::filesystem;

namespace utils {

/* Return true if any element is shared between two sets. */
template <typename Set>
inline bool any_intersection(const Set &a, const Set &b)
{
    return std::find_first_of(a.cbegin(), a.cend(), b.cbegin(), b.cend()) != a.cend();
}

/* Check if the path is a valid download directory. */
std::error_code validate_download_dir(const fs::path &path);

string vector_to_string(const vector<string> &vec);
vector<string> split_string(const string &str);

/* Check if the given path is a file and can be read. */
bool readable_file(const fs::path &path);

/* Generate a Lorem Ipsum string. */
string lipsum(int repeats);

/*
 * Return a rounded percentage in the range [0,100]
 * from a domain of [0.0,1.0]
 */
int percent_round(double d);

/* ns utils */
}
