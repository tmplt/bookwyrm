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

/*
 * A simple wrapper around a dymaic C-array with ITEM pointers.
 * Initial capacity is 50. Capacity if multiplied with 2 when needed.
 */
class item_array {
public:
    explicit item_array();
    ~item_array();

    /* Append an ITEM* to the array, and push the null-terminator forward. */
    void append_new_item(const char *name, const char *desc);

    ITEM** operator*()
    {
        return items_;
    }

private:
    ITEM **items_;
    size_t null_idx_;
};

class menu {
public:
    explicit menu(vector<item> &items);
    ~menu();

    /* Fires up the menu after necessary curses configuration. */
    void display();

    /* Updates the menu entries to match those in items_. */
    void update();

private:
    /* new_item-fications of the elements in items_. */
    /* ITEM **menu_items_; */
    /* int null_idx; */
    item_array menu_items_;

    MENU *menu_;
    ITEM *current_;
    std::mutex menu_mutex_;

    /* Reference from parent class. */
    vector<item> const &items_;
};

} /* ns bookwyrm */
