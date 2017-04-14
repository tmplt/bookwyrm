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

#include <array>

#include "fuzzywuzzy.hpp"

#include "item.hpp"
#include "utils.hpp"
#include "common.hpp"
#include "algorithm.hpp"

static constexpr int fuzzy_min = 75;

/*
 * Returns true if all specified exact values are equal
 * and if all specified non-exact values passes the fuzzy ratio.
 */
bool bookwyrm::item::matches(const item &wanted)
{
    /* Return false if any exact value doesn't match what's wanted. */
    for (int i = 0; i <= wanted.exacts.size; i++) {
        if (wanted.exacts[i] && wanted.exacts[i] != this->exacts[i])
            return false;
    }

    /* Does the item contain a wanted ISBN? */
    if (!wanted.misc.isbns.empty() &&
            !utils::any_intersection(wanted.misc.isbns, this->misc.isbns))
        return false;

    /* Are we copying the strings here? */
    const std::array<string, 3> in_result = {this->nonexacts.title,
                                             this->nonexacts.serie,
                                             this->nonexacts.publisher},
                                requested = {wanted.nonexacts.title,
                                             wanted.nonexacts.serie,
                                             wanted.nonexacts.publisher};

    for (size_t i = 0; i <= in_result.size(); i++) {
        if (!requested[i].empty()) {
            /*
             * partial: useful for course literature that can have some
             * crazy long titles. Also useful for publishers, because
             * some entries may not use the full name.
             */
            if (fuzz::partial_ratio(in_result[i], requested[i]) < fuzzy_min)
                return false;
        }
    }

    if (!wanted.nonexacts.authors.empty()) {
        int max_ratio = 0;
        for (const auto &comb : algorithm::product(this->nonexacts.authors,
                    wanted.nonexacts.authors)) {
            /*
             * From some quick testing, it feels like token_set_ratio
             * works best here.
             */
            int ratio = fuzz::token_set_ratio(comb.first, comb.second);
            max_ratio = std::max(ratio, max_ratio);

            if (max_ratio >= fuzzy_min)
                break;
        }

        if (max_ratio < fuzzy_min)
            return false;
    }

    return true;
}
