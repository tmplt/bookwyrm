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

#include <utility>
#include <vector>
#include <algorithm>

#include "item.hpp"

namespace func {

/*
 * Take two containers, zip over them and return a vector with the length of the shortest passed container.
 *
 * NOTE: This is some ugly template programming. Please fix it.
 */
template <typename Cont1, typename Cont2>
auto zip(const Cont1 &ac, const Cont2 &bc) -> std::vector<std::pair<typename Cont1::value_type, typename Cont2::value_type>>
{
    std::vector<std::pair<typename Cont1::value_type, typename Cont2::value_type>> pairs;
    auto a = std::cbegin(ac);
    auto b = std::cbegin(bc);

    while (a != std::cend(ac) && b != std::cend(bc))
        pairs.emplace_back(*a++, *b++);

    assert(pairs.size() == std::min(ac.size(), bc.size()));

    return pairs;
}

/* ns func */
}
