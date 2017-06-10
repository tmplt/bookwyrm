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

#include <menu.h>
#include <vector>
using std::vector; // including common.hpp here breaks errors.hpp?
#include <mutex>

#include "item.hpp"

namespace bookwyrm {

class menu {
public:
    explicit menu(vector<item> &items);
    ~menu();

    void display();

    /* should be called after every searcher::append_item. */
    void update();

private:
    /* const vector<item> &items_; */
    vector<ITEM*> menu_items_;
    MENU *menu_;
    std::mutex menu_mutex_;

    vector<item> &items_;
};

// use an integer to save which item to highlight

} /* ns bookwyrm */
