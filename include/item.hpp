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

#include <pybind11/pybind11.h>
#include <tuple>

#include "common.hpp"
#include "storage.hpp"
#include "components/command_line.hpp"

namespace py = pybind11;

namespace bookwyrm {

class item {
public:
    explicit item(const std::unique_ptr<cliparser> &cli)
        : nonexacts(cli), exacts(cli) {};
    /* TODO: Move construct this */
    explicit item(const std::tuple<nonexacts_t, exacts_t> &tuple)
        : nonexacts(std::get<0>(tuple)), exacts(std::get<1>(tuple)) {};

    bool matches(const item &wanted);

    friend std::ostream& operator<<(std::ostream &os, item const &i)
    {
        os << "test printout: " + i.nonexacts.serie;
        return os;
    }

    /*
     * These ought to be const, but since we're binding
     * these in Python as well, we have two choices:
     *  i) make these const and instead bind constructors
     *     taking a py::object of some sort, relying on
     *     multiple callbacks to C++ code before the item
     *     has been created wholly.
     * ii) make these non-const, thus allowing us to assign
     *     each field as we want in Python code.
     *
     * Alternatively, instead of giving back an actual
     * item to C++, we could return a tuple of three dicts:
     *     (
     *         {'authors' : ['a', 'b', ...], 'title' : 'whatev', ...},
     *         {'year' : 1990, 'edition' : 3, ...},
     *         {'isbns' : ['123123', ...], ...}
     *     )
     * and emplace the new item here in C++. No need to bind
     * them item class, then, either.
     */
    nonexacts_t nonexacts;
    exacts_t exacts;
    misc_t misc;
};

/* ns bookwyrm */
}
