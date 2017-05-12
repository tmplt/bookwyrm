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
#include <spdlog/spdlog.h>

#include "components/logger.hpp"
#include "common.hpp"
#include "item.hpp"
#include "errors.hpp"

namespace bookwyrm {

/*
 * This class will handle everything between the
 */
class searcher {
public:
    explicit searcher(const item &wanted);

    void search();

    void append_item(std::tuple<nonexacts_t, exacts_t> item_comps)
    {
        items_.emplace_back(item_comps);

        /* update the menu here */
    }

private:
    const logger_t _logger = spdlog::get("main");
    const item &wanted_;
    vector<item> items_;
    vector<pybind11::module> sources_;
};

/* ns bookwyrm */
}
