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
#include <mutex>

#include "components/logger.hpp"
#include "components/menu.hpp"
#include "common.hpp"
#include "item.hpp"

namespace bookwyrm {

/*
 * TODO: finish this sentence.
 * This class will handle everything between the
 */
class searcher {
public:
    explicit searcher(const item &wanted);

    /*
     * Explicitly delete the copy-constructor.
     * Doing this allows us to run each python
     * module in its own thread.
     *
     * This might be because threads_ is considered
     * copy-constructible, and when passing this
     * to the Python module, a copy is wanted instead
     * of a reference.
     */
    searcher(const searcher&) = delete;
    ~searcher();

    searcher& async_search();
    void display_menu();

    void append_item(std::tuple<nonexacts_t, exacts_t> item_comps)
    {
        item item(item_comps);
        /* if (!item.matches(wanted_)) */
            /* return; */

        std::lock_guard<std::mutex> guard(items_mutex_);
        items_.push_back(item);
        menu_.update();
    }

private:
    const logger_t _logger = spdlog::get("main");
    const item &wanted_;
    vector<item> items_;
    std::mutex items_mutex_;

    vector<pybind11::module> sources_;
    vector<std::thread> threads_;

    menu menu_{items_};
};

/* ns bookwyrm */
}
