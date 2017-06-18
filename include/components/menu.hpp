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

#include <vector>
using std::vector; // including common.hpp here breaks errors.hpp?
#include <mutex>

#include "item.hpp"

namespace bookwyrm {

class menu {
public:
    explicit menu(vector<item> &items)
        : items_(items), y_(0), selected_item_(0) {}

    ~menu();

    /* Fires up the menu. */
    void display();

    /* Updates the menu entries to match those in items_. */
    void update();

private:
    std::mutex menu_mutex_;

    /* Reference from parent class. */
    vector<item> const &items_;

    size_t item_count()
    {
        return items_.size();
    }

    /*                         */
    /* For printing the items. */
    /*                         */

    /* Current y-coordinate on the terminal. */
    int y_;

    /* Index of the currently selected item. */
    int selected_item_;

    /* Prints an item on the current y-coordinate. */
    void print_item(const item &t);

    // ncurses-esque functions
    void mvprintw(int x, int y, string str);
};

} /* ns bookwyrm */
