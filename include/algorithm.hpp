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

#include "common.hpp"

namespace algorithm {

/*
 * Generates the cartesian product between two vectors.
 */
template <class T1, class T2>
const auto product(const vector<T1> &a, const vector<T2> &b)
{
    /* NOTE: are we copying the references or the full strings here? */
    vector<std::pair<T1, T2>> products;

    for (const auto &first : a) {
        for (const auto &second : b)
            products.emplace_back(first, second);
    }

    return products;
}

}
