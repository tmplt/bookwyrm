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

/* Check if the given path is a file and can be read. */
inline bool valid_file(const fs::path &path)
{
    return fs::is_regular_file(path) && access(path.c_str(), R_OK) == 0;
}

/* ns utils */
}
