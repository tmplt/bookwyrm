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
constexpr int empty = -1;

/*
 * For --year:
 *   -y 2157   : list items from 2157 (equal)
 *   -y =>2157 : list items from 2157 and later (eq_gt)
 *   -y =<2157 : list items from 2157 and earlier (eq_lt)
 *   -y >2157  : list items from later than 2157 (gt)
 *   -y <2157  : list items from earlier than 2157 (lt)
 */
enum class year_mod { equal, eq_gt, eq_lt, lt, gt, unused };

struct exacts_t {
    /* Holds exact data about an item (year, page count, format, etc.). */
    explicit exacts_t(const cliparser &cli)
        : exacts_t{get_yearmod(cli), cli} {}

    explicit exacts_t(const std::map<string, int> &dict, const string &extension)
        : ymod(year_mod::unused),
        year(get_value(dict, "year")),
        volume(get_value(dict, "volume")),
        number(get_value(dict, "number")),
        pages(get_value(dict, "pages")),
        size(get_value(dict, "size")),
        extension(extension) {}

    const year_mod ymod;
    const int year,
              volume,  /* no associated flag */
              number,  /* no associated flag */
              pages,   /* no associated flag */
              size;    /* in bytes; no associated flag */

    const string extension;

    /* Convenience container */
    const std::array<int, 5> store = {{
        year, volume, number, pages, year
    }};

private:
    static int parse_number(const cliparser &cli, const string &&opt);
    static int get_value(const std::map<string, int> &dict, const string &&key);

    /* Parse the year which may have a prefixed modifier. */
    static const std::pair<year_mod, int> get_yearmod(const cliparser &cli);

    explicit exacts_t(const std::pair<year_mod, int> &pair, const cliparser &cli)
        : ymod(std::get<0>(pair)), year(std::get<1>(pair)),
        volume(parse_number(cli, "volume")),
        number(parse_number(cli, "number")),
        pages(empty),
        size(empty),
        extension(cli.get("extension")) {}
};

struct nonexacts_t {
    /* Holds strings, which are matched fuzzily. */
    explicit nonexacts_t(const cliparser &cli)
        : authors(cli.get_many("author")),
        title(cli.get("title")),
        series(cli.get("series")),
        publisher(cli.get("publisher")),
        journal(cli.get("journal")) {}

    explicit nonexacts_t(const std::map<string, string> &dict, const vector<string> &authors)
        : authors(authors),
        title(get_value(dict, "title")),
        series(get_value(dict, "series")),
        publisher(get_value(dict, "publisher")),
        journal(get_value(dict, "journal")),
        edition(get_value(dict, "edition")) {}

    const vector<string> authors;
    const string title;
    const string series;
    const string publisher;
    const string journal;
    const string edition;

private:
    static const string get_value(const std::map<string, string> &dict, const string &&key);
};

struct request {
    /* Holds necessary data to download an item. */

    /* some type enum? ´headers´ will only be used when the mirror is over HTTP. */
    const string uri;
    const std::map<string, string> headers;
}

struct misc_t {
    /* Holds everything else. */
    explicit misc_t(const vector<string> &uris, const vector<string> &isbns)
        : uris(uris), isbns(isbns) {}
    explicit misc_t() {} // cannot be initialized from cli options

    const vector<string> uris;
    const vector<string> isbns;
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
    const misc_t misc;
};

/* ns bookwyrm */
}
