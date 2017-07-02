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
    explicit exacts_t(const std::unique_ptr<cliparser> &cli);
    explicit exacts_t(const std::map<string, int> &dict);

    year_mod ymod;

    int year,
        edition,
        format, // unused for now
        volume,
        number,
        pages,  // missing flag
        lang;   // unused for now

    string year_str;

    constexpr static int size = 7;

    std::array<int, size> store = {
        year, edition, format,
        volume, number, pages
    };

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
    explicit nonexacts_t(const std::unique_ptr<cliparser> &cli);
    explicit nonexacts_t(const std::map<string, string> &dict, const vector<string> &authors);

    vector<string> authors;
    string authors_str;
    string title;
    string serie;
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
    explicit item(const std::unique_ptr<cliparser> &cli)
        : nonexacts(cli), exacts(cli) {}

    /* Construct an item from a pybind11::tuple. */
    explicit item(const std::tuple<nonexacts_t, exacts_t> &tuple)
        : nonexacts(std::get<0>(tuple)), exacts(std::get<1>(tuple)) {}

    bool matches(const item &wanted);

    friend std::ostream& operator<<(std::ostream &os, item const &i)
    {
        os << "test printout: " + i.nonexacts.serie;
        return os;
    }

    const string menu_order(size_t idx) const
    {
        /*
         * This isn't very good structure. I'd rather have a vector
         * of string references (reference_wrapped) and just grab by index,
         * but for some bloody reason those cannot be initialized with a
         * brace-enclosed initializer list.
         */
        switch (idx) {
            case 0:  return nonexacts.title;
            case 1:  return exacts.year_str;
            case 2:  return nonexacts.serie;
            case 3:  return nonexacts.authors_str;
            case 4:  return nonexacts.publisher;
            case 5:  return "format (TODO)";
            default: assert(false);
        }
    }

    const nonexacts_t nonexacts;
    const exacts_t exacts;
    misc_t misc;
};

/* ns bookwyrm */
}
