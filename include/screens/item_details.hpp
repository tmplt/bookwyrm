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

#include "item.hpp"
#include "screens/base.hpp"

// namespace this to bookwyrm::screens and then have it as bookwyrm::screens::details?

/*
 * Interface-wise, this will be like opening an email for reading in mutt.
 * A thread will be spawned to fetch more info about the item from some database,
 * and the bookwyrm will print that info in this (overlapping) window in a pretty way.
 *
 * The user should still be able to check another item's details while this thread is running.
 * Item details will be kept in the actual item. So a passed item will be modified, otherwise,
 * if the user goes back to an item, we'll need to fetch the data again.
 *
 * The user doesn't need to exit the detail screen to select another item.
 * Implementing this is a problem for the future, though.
 */
namespace screen {

class item_details : private base {
public:
    explicit item_details(bookwyrm::item &t, int pad_top)
        : base(pad_top, 3, 0, 0), item_(t) { }

    void display();
    void update();
    void on_resize();

private:
    const bookwyrm::item &item_;
    /* void print_border(); */
};

/* ns screen */
}

