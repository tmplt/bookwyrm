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

#include <mutex>
#include <memory>
#include <spdlog/spdlog.h>

#include "common.hpp"
#include "item.hpp"
#include "python.hpp"
#include "components/logger.hpp"
#include "screens/multiselect_menu.hpp"

namespace bookwyrm {

class multiselect_menu;

/*
 * This class handles the threads running the source scirpts.
 * Upon construction, all valid scripts are found and loaded.
 * Upon async_search(), each script is run in a seperate thread.
 * These threads will call add_item() during run_time.
 */
class searcher {
public:

    /* Search for valid script modules and store them. */
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
    void async_search();

    /* Add a found item to items_ and update the menu. */
    void add_item(std::tuple<nonexacts_t, exacts_t> item_comps);

    vector<item>& results()
    {
        return items_;
    }

    void set_menu(std::shared_ptr<multiselect_menu> m)
    {
        menu_ = m;
    }

private:
    const logger_t logger_ = spdlog::get("main");
    const item &wanted_;

    /* Somewhere to store our found items. */
    vector<item> items_;

    /* A lock for when multiple threads want to add an item. */
    std::mutex items_mutex_;

    /* The valid Python modules we have found. */
    vector<pybind11::module> sources_;

    /* The same Python modules, but now running! */
    vector<std::thread> threads_;

    /* Which menu do we want to notify about updates? */
    std::shared_ptr<multiselect_menu> menu_;
};

/* ns bookwyrm */
}
