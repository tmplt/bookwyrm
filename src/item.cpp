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

#include <cctype>

#include <fuzzywuzzy.hpp>

#include "item.hpp"
#include "utils.hpp"
#include "common.hpp"
#include "algorithm.hpp"
#include "functional.hpp"

static constexpr int fuzzy_min = 75;

namespace bookwyrm {

int exacts_t::parse_number(const cliparser &cli, const string &&opt)
{
    const auto value_str = cli.get(opt);
    if (value_str.empty()) return empty;

    try {
        return std::stoi(value_str);
    } catch (std::exception &err) {
        throw value_error("malformed value '" + value_str + "' for argument --" + opt);
    }
}

int exacts_t::get_value(const std::map<string, int> &dict, const string &&key)
{
    const auto elem = dict.find(key);
    return elem == dict.cend() ? empty : elem->second;
}

const std::pair<year_mod, int> exacts_t::get_yearmod(const cliparser &cli)
{
    const auto year_str = cli.get("year");
    if (year_str.empty()) return {year_mod::equal, empty};

    const auto start = std::find_if(year_str.cbegin(), year_str.cend(), [](char c) {
        return std::isdigit(c);
    });

    try {
        /*
         * NOTE: this approach allows the year to be represented as a float
         * (which stoi truncates to an int) and allows appended not-digits.
         * Will this cause problems?
         */
        const auto year = std::stoi(string(start, year_str.cend()));

        if (start != year_str.cbegin()) {
            /* There is a modifier in front of the year */
            string mod_str(year_str.cbegin(), start);
            year_mod mod;

            if (mod_str == "=>")
                mod = year_mod::eq_gt;
            else if (mod_str == "=<")
                mod = year_mod::eq_lt;
            else if (mod_str == ">")
                mod = year_mod::gt;
            else if (mod_str == "<")
                mod = year_mod::lt;
            else
                throw value_error("unrecognised year modifier '" + mod_str + '\'');

            return {mod, year};
        }

        return {year_mod::equal, year};

    } catch (const value_error &err) {
        throw err;
    } catch (const std::exception &err) {
        throw value_error("malformed year");
    }
}

const string nonexacts_t::get_value(const std::map<string, string> &dict, const string &&key)
{
    const auto elem = dict.find(key);
    return elem == dict.cend() ? "" : elem->second;
}

bool item::matches(const item &wanted) const
{
    // TODO: implement operator== for exacts_t?

    /* Return false if any exact value doesn't match what's wanted. */
    for (const auto& [req, got] : func::zip(wanted.exacts.store, this->exacts.store)) {
        if (req != empty && req != got)
            return false;
    }

    /* Ad-hoc the file type, for now. */
    if (!wanted.exacts.extension.empty()) {
        if (this->exacts.extension != wanted.exacts.extension)
            return false;
    }

    /* Does the item contain a wanted ISBN? */
    if (!wanted.misc.isbns.empty() &&
            !utils::any_intersection(wanted.misc.isbns, this->misc.isbns))
        return false;

    const std::array<string, 3> in_result = {{
                                    this->nonexacts.title,
                                    this->nonexacts.series,
                                    this->nonexacts.publisher
                                }},
                                requested = {{
                                    wanted.nonexacts.title,
                                    wanted.nonexacts.series,
                                    wanted.nonexacts.publisher
                                }};

    for (const auto& [req, got] : func::zip(requested, in_result)) {
        if (!req.empty()) {
            /*
             * partial: useful for course literature that can have some
             * crazy long titles. Also useful for publishers, because
             * some entries may not use the full name.
             */
            if (fuzz::partial_ratio(got, req) < fuzzy_min)
                return false;
        }
    }

    if (!wanted.nonexacts.authors.empty()) {
        int max_ratio = 0;
        for (const auto& [req, got] : algorithm::product(wanted.nonexacts.authors,
                    this->nonexacts.authors)) {
            /*
             * From some quick testing, it feels like token_set_ratio
             * works best here.
             */
            int ratio = fuzz::token_set_ratio(req, got);
            max_ratio = std::max(ratio, max_ratio);

            if (max_ratio >= fuzzy_min)
                break;
        }

        if (max_ratio < fuzzy_min)
            return false;
    }

    return true;
}

/* ns bookwyrm */
}
