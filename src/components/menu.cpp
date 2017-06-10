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

void menu::construct(const vector<item> &items)
{
    std::lock_guard<std::mutex> guard(menu_mutex_);

    initscr();
    clear();

    /*
     * Don't echo the pressed characters back,
     * and let us read individual keys.
     */
    noecho(); cbreak();

    /*
     * We also want to read special keys.
     */
    keypad(stdscr, true);

    /* Hide the cursor. */
    if (curs_set(0) == ERR)
        spdlog::get("main")->warn("curses: can't hide the cursor");

    /* Initialize the items. */
    for (const auto &item : items) {
        menu_items_.push_back(
            new_item(item.nonexacts.title.c_str(), "desc")
        );
    }

    menu_ = new_menu(const_cast<ITEM**>(menu_items_.data()));
    mvprintw(LINES - 2, 0, "Press 'q' to quit");
}

menu::~menu()
{
    /* Deconstruct everything, and quick curses. */
    unpost_menu(menu_);
    for (auto &item : menu_items_) free_item(item);
    free_menu(menu_);

    endwin();
}

void menu::display()
{
    post_menu(menu_);

    menu_mutex_.lock();
    refresh();
    menu_mutex_.unlock();

    char c;
    while ((c = getch()) != 'q') {
        switch (c) {
            case 'j':
                menu_driver(menu_, REQ_DOWN_ITEM);
                break;
            case 'k':
                menu_driver(menu_, REQ_UP_ITEM);
                break;
            case 'g':
                menu_driver(menu_, REQ_FIRST_ITEM);
                break;
            case 'G':
                menu_driver(menu_, REQ_LAST_ITEM);
                break;
        }
    };
}

void menu::update()
{
    std::lock_guard<std::mutex> guard(menu_mutex_);

    mvprintw(0, 0, "update() has been called!");
    refresh();
}

} /* ns bookwyrm */
