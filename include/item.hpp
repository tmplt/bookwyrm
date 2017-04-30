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

#include "storage.hpp"
#include "components/command_line.hpp"

using std::string;
using std::vector;

namespace bookwyrm {

class item {
public:
    explicit item(const std::unique_ptr<cliparser> &cli)
        : nonexacts(cli), exacts(cli) {};

    bool matches(const item &wanted);

    const nonexacts_t nonexacts;
    const exacts_t exacts;
    misc_t misc;
};

/* ns bookwyrm */
}
