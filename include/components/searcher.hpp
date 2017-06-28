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

#include <spdlog/spdlog.h>
#include <mutex>

#include "common.hpp"
#include "item.hpp"
#include "python.hpp"
#include "components/logger.hpp"
#include "screens/multiselect_menu.hpp"

namespace bookwyrm {

/*
 * The more monolothic class of this program.
 * This class handles everything from starting the
 * Python threads to getting the selected items
 * the user wants.
 *
 * TODO: this name doesn't make much sense any more.
 * Find a better one.
 */
class searcher {
public:

    /* Search for valid Python modules and store them. */
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

    /* Start a std::thread for each valid Python module found. */
    searcher& async_search();

    /* Display the menu and let the user select which items to download. */
    void display_menu();

    /* Add a found item to items_ and update the menu. */
    void add_item(std::tuple<nonexacts_t, exacts_t> item_comps);

private:
    const logger_t logger_ = spdlog::get("main");
    const item &wanted_;
    vector<item> items_;
    std::mutex items_mutex_;

    vector<pybind11::module> sources_;
    vector<std::thread> threads_;

    multiselect_menu menu_{items_};
};

/* ns bookwyrm */
}
