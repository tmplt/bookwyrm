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

#include <algorithm>

namespace utils {

/* Is any elements shared between the two sets? */
template <typename Set>
bool any_match(Set a, Set b)
{
    std::sort(a.begin(), a.end());
    std::sort(b.begin(), b.end());

    bool match = std::find_first_of(a.begin(), a.end(),
                                    b.begin(), b.end()) != a.end();

    return match;
}

/* ns utils */
}
