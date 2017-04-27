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
#include <cctype>

#include <fuzzywuzzy.hpp>

#include "item.hpp"
#include "utils.hpp"
#include "common.hpp"
#include "algorithm.hpp"
#include "storage.hpp"
#include "errors.hpp"
#include "common.hpp"

static constexpr int fuzzy_min = 75;

namespace bookwyrm {

item::item(const std::unique_ptr<cliparser> &cli)
{
    /* Parse the year which may have a prefixed modifier. */
    std::tie(exacts.year_mod, exacts.year) = [&cli]() -> std::pair<int, int> {
        const auto year_str = cli->get("year");
        if (year_str.empty()) return {empty, empty};

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
                int mod;

                if (mod_str == "=>")
                    mod = eq_gt;
                else if (mod_str == "=<")
                    mod = eq_lt;
                else if (mod_str == ">")
                    mod = gt;
                else if (mod_str == "<")
                    mod = lt;
                else
                    throw value_error("unrecognized year modifier '" + mod_str + '\'');

                return {mod, year};
            }

            return {equal, year};
        } catch (const value_error &err) {
            throw err;
        } catch (const std::exception &err) {
            throw value_error("malformed year");
        }
    }();

    auto parse_number = [&cli](string &&opt) -> int {
        auto value_str = cli->get(opt);
        if (value_str.empty()) return empty;

        try {
            return std::stoi(value_str);
        } catch (std::exception &err) {
            throw value_error("malformed value '" + value_str + "' for argument --" + opt);
        }
    };

    exacts.edition = parse_number("edition");
    exacts.volume  = parse_number("volume");
    exacts.number  = parse_number("number");
    exacts.pages   = parse_number("pages");

    nonexacts.authors   = cli->get_many("author");
    nonexacts.title     = cli->get("title");
    nonexacts.serie     = cli->get("serie");
    nonexacts.publisher = cli->get("publisher");
    nonexacts.journal   = cli->get("journal");

    misc.isbns = cli->get_many("isbn");
}

/*
 * Returns true if all specified exact values are equal
 * and if all specified non-exact values passes the fuzzy ratio.
 */
bool item::matches(const item &wanted)
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

    /* NOTE: are we copying the strings here? */
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

/* ns bookwyrm */
}
