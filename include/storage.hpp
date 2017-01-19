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

#include <vector>
#include <string>

using std::vector;
using std::string;

namespace bookwyrm {

/* Default value: "this value is empty". */
enum { empty = -1 };

struct exacts_t {
    using store_t = vector<int>;
    /*
     * A container with members that we can iterate over.
     * Useful in item::matches() where we want to iterate
     * over these values and check it they match.
     *
     * We also need to get these values by name, which is
     * why we don't use a vector.
     */

    int year    = empty;
    int edition = empty;
    int ext     = empty;
    int volume  = empty;
    int number  = empty;
    int pages   = empty;
    int lang    = empty;

    constexpr static int size = 7;

    store_t store = {
        year, edition, ext,
        volume, number, pages
    };

    int operator[](int i) const
    {
        return store[i];
    }

    /* For complience for for-each loops. */
    store_t::iterator begin()
    {
        return store.begin();
    }

    store_t::const_iterator begin() const
    {
        return store.begin();
    }

    store_t::iterator end()
    {
        return store.end();
    }
    store_t::const_iterator end() const
    {
        return store.end();
    }
};

struct nonexacts_t {
    vector<string> authors;
    string title;
    string serie;
    string publisher;
    string journal;
};

struct misc_t {
    vector<string> isbns;
    vector<string> mirrors;
};

/* ns bookwyrm */
}
