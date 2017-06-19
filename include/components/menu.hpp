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

#include <set>
#include <mutex>
#include <termbox.h>

#include "common.hpp"
#include "item.hpp"

namespace bookwyrm {

class menu {
public:
    explicit menu(vector<item> &items)
        : y_(0), selected_item_(0), scroll_offset_(0),
        items_(items) {}

    ~menu();

    /* Fires up the menu. */
    void display();

    /* Updates the menu entries to match those in items_. */
    void update();

private:
    enum move_direction { up, down };

    /* Current y-coordinate on the terminal. */
    int y_;

    /* Index of the currently selected item. */
    int selected_item_;

    /* How many lines have we scrolled? */
    int scroll_offset_;

    std::mutex menu_mutex_;
    vector<item> const &items_;

    /* Item indices marked for download. */
    std::set<int> marked_items_;

    size_t item_count() const
    {
        return items_.size();
    }

    bool is_marked(size_t idx) const
    {
        return marked_items_.find(idx) != marked_items_.cend();
    }

    /* How many entries can the menu print in the terminal? */
    int menu_capacity() const
    {
        // -3 is arbitrary for now
        return tb_height() - 3;
    }

    /*
     * Is the currently selected item the last one in the
     * menu "window"?
     */
    bool menu_at_bot() const
    {
        return selected_item_ == (menu_capacity() - 1 + scroll_offset_);
    }

    /*
     * Is the currently selected item the first one in the
     * menu "window"?
     */
    bool menu_at_top() const
    {
        return selected_item_ == scroll_offset_;
    }

    /* Prints an item on the current y-coordinate. */
    void print_item(const item &t);

    /* From Ncurses. */
    void mvprintw(int x, int y, string str);

    /* Move up and down the menu. */
    void move(move_direction dir);

    /* Select (or unselect) the current item for download. */
    void toggle_select();

    void mark_item(size_t idx)
    {
        marked_items_.insert(idx);
    }

    void unmark_item(size_t idx)
    {
        marked_items_.erase(idx);
    }
};

} /* ns bookwyrm */
