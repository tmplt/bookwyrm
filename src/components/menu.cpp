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

/* Planned behavior:
 *   alike vim, when the currently selected object is but a few
 *   items down or up, and the menu is scrollable, scroll a single
 *   line in the same direction. This can be donw with REQ_SCR_*.
 *
 *   'g' to go to the topmost item (REQ_FIRST_ITEM).
 *
 *   'G' to go to the bottommost item (REQ_LAST_ITEM).
 *
 *   '<spacebar> to select an item (REQ_TOGGLE_ITEM).
 */

#include <spdlog/spdlog.h>

#include "components/menu.hpp"
#include "components/logger.hpp"

namespace bookwyrm {

menu::menu(vector<item> items)
{
    initscr();
    clear();

    /*
     * Don't echo the pressed characters back,
     * and let us read individual keys.
     */
    echo(); cbreak();

    /*
     * We also want to read special keys.
     */
    keypad(stdscr, true);

    /* Hide the cursor. */
    if (curs_set(0) == ERR) {
        spdlog::get("main")->warn("curses: can't hide the cursor");
    }

    /* Initialize the items. */
    for (auto &item : items) {
        items_.push_back(new_item(
                item.nonexacts.title.c_str(),
                item.nonexacts.title.c_str()
        ));
    }
    items_.emplace_back(nullptr);

    auto menu = new_menu(const_cast<ITEM**>(items_.data()));
    post_menu(menu);
    refresh();

    char c;
    while ((c = getch()) != 'q') {
        switch (c) {
            case 'j':
                menu_driver(menu, REQ_DOWN_ITEM);
                break;
            case 'k':
                menu_driver(menu, REQ_UP_ITEM);
        }
    };

    unpost_menu(menu);
    for (auto &item : items_) free_item(item);
    free_menu(menu);
}

menu::~menu()
{
    /* deconstruct all windows here, if any */
    /* for (auto &item : items_) free_item(item); */

    endwin();
}

} /* ns bookwyrm */
