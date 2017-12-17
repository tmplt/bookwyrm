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

#include <cassert>
#include <array>
#include <tuple>

#include "common.hpp"
#include "utils.hpp"
#include "components/command_line.hpp"

// TODO: clean theese up and set everything in stone (don't store strings, file extensions, etc.).
// and make everything const

namespace bookwyrm {

/* Default value: "this value is empty". */
enum { empty = -1 };

/*
 * For --year:
 *   -y 2157   : list items from 2157 (equal)
 *   -y =>2157 : list items from 2157 and later (eq_gt)
 *   -y =<2157 : list items from 2157 and earlier (eq_lt)
 *   -y >2157  : list items from later than 2157 (gt)
 *   -y <2157  : list items from earlier than 2157 (lt)
 */
enum class year_mod { equal, eq_gt, eq_lt, lt, gt };

struct exacts_t {
    /* Holds exact data about an item (year, page count, format, etc.). */
    explicit exacts_t(const cliparser &cli);
    explicit exacts_t(const std::map<string, int> &dict);

    year_mod ymod;
    int year = empty;

    int edition = empty,
        volume  = empty,  /* no associated flag */
        number  = empty,  /* no associated flag */
        pages   = empty;  /* no associated flag */

    string format = "";

    /* Convenience container */
    std::array<int, 6> store = {{
        year, edition, volume, number, pages
    }};
};

struct nonexacts_t {
    /* Holds strings, which are matched fuzzily. */
    explicit nonexacts_t(const cliparser &cli);
    explicit nonexacts_t(const std::map<string, string> &dict, const vector<string> &authors);

    vector<string> authors;
    string title;
    string series;
    string publisher;
    string journal;
};

struct misc_t {
    /*
     * The POD for everything else and undecided
     * fields.
     */
    explicit misc_t(const vector<string> &uris);
    explicit misc_t() {}; // cannot be initialized from cli options

    vector<string> isbns;
    vector<string> uris;
};

class item {
public:
    explicit item(const cliparser &cli)
        : nonexacts(cli), exacts(cli) {}

    /* Construct an item from a pybind11::tuple. */
    explicit item(const std::tuple<nonexacts_t, exacts_t, misc_t> &tuple)
        : nonexacts(std::get<0>(tuple)), exacts(std::get<1>(tuple)), misc(std::get<2>(tuple)) {}

    /*
     * Returns true if all specified exact values are equal
     * and if all specified non-exact values passes the fuzzy ratio.
     */
    bool matches(const item &wanted) const;

    const nonexacts_t nonexacts;
    const exacts_t exacts;
    misc_t misc;
};

/* ns bookwyrm */
}
