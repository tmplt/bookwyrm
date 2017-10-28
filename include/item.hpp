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
    /*
     * A POD with added index operator.
     * Useful in item::matches() where we want to
     * iterate over these values and check if they match.
     *
     * We can then get a field value by name, which we'll
     * want when printing the stuff out.
     *
     * Fields are set to "empty" (-1) during construction.
     * This makes us able to bool-check (since -1 is false)
     * whether or not a field is empty or not.
     */
    explicit exacts_t(const cliparser &cli);
    explicit exacts_t(const std::map<string, int> &dict);

    year_mod ymod;
    int year = empty;
    string year_str = "";

    int edition = empty,
        volume  = empty,  /* no associated flag */
        number  = empty,  /* no associated flag */
        pages   = empty;  /* no associated flag */

    string format = "fmt";

    constexpr static int size = 6;
    std::array<int, size> store = {{
        year, edition, volume, number, pages
    }};

    int operator[](int i) const
    {
        return store[i];
    }
};

struct nonexacts_t {
    /*
     * As the typename suggests, this POD contains
     * data that we're not going to match exacly match
     * exactly with the wanted field. Instead, we use
     * fuzzy-matching.
     */
    explicit nonexacts_t(const cliparser &cli);
    explicit nonexacts_t(const std::map<string, string> &dict, const vector<string> &authors);

    vector<string> authors;
    string authors_str;
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

    vector<string> isbns;
    vector<string> mirrors;
};

class item {
public:
    explicit item(const cliparser &cli)
        : nonexacts(cli), exacts(cli) {}

    /* Construct an item from a pybind11::tuple. */
    explicit item(const std::tuple<nonexacts_t, exacts_t> &tuple)
        : nonexacts(std::get<0>(tuple)), exacts(std::get<1>(tuple)) {}

    /*
     * Returns true if all specified exact values are equal
     * and if all specified non-exact values passes the fuzzy ratio.
     */
    bool matches(const item &wanted) const;

    friend std::ostream& operator<<(std::ostream &os, item const &i)
    {
        os << "test printout: " + i.nonexacts.series;
        return os;
    }

    const nonexacts_t nonexacts;
    const exacts_t exacts;
    misc_t misc;
};

/* ns bookwyrm */
}
