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

#include "item.hpp"
#include "utils.hpp"

using std::vector;
using std::string;

/*
 * Returns true if all specified exact values are equal
 * and if all specified non-exact values passed the fuzzy ratio.
 */
bool bookwyrm::item::matches(const item &wanted)
{
    /* Return false if any exact value doesn't match what's wanted. */
    for (auto i = 0; i <= wanted.exacts.size; i++) {
        if (wanted.exacts[i] &&
                wanted.exacts[i] != this->exacts[i])
            return false;
    }

    /* Does the item contain a wanted ISBN? */
    if (!wanted.misc.isbns.empty() &&
            !utils::any_match(wanted.misc.isbns, this->misc.isbns))
        return false;

    /* TODO: Check nonexacts and authors. */

    return true;
}
