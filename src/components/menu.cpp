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

#include <fmt/format.h>
#include <termbox.h>
#include <spdlog/spdlog.h>
#include <pybind11/embed.h>

#include "errors.hpp"
#include "components/menu.hpp"
#include "components/logger.hpp"

namespace py = pybind11;

namespace bookwyrm {

menu::~menu()
{
    tb_shutdown();
}

void menu::display()
{
    menu_mutex_.lock();

    int code = tb_init();
    if (code < 0) {
        string err = "termbox init failed with code: " + code;
        throw component_error(err.data());
    }

    tb_set_cursor(TB_HIDE_CURSOR, TB_HIDE_CURSOR);
    tb_clear();

    menu_mutex_.unlock();

    /*
     * Let the source threads free.
     * This doesn't feel like the best place to have this,
     * but we do need to have a release in scope if we
     * ever want the threads to run.
     * TODO: find out if there is a better place for this.
     */
    py::gil_scoped_release nogil;

    struct tb_event ev;
    bool quit = false;
    while (tb_poll_event(&ev) && !quit) {
        if (ev.type == TB_EVENT_KEY) {
            switch (ev.key) {
                case TB_KEY_ESC:
                    quit = true;
                    break;
                case TB_KEY_ARROW_DOWN:
                    move(down);
                    break;
                case TB_KEY_ARROW_UP:
                    move(up);
                    break;
            }
        }
    }
}

void menu::update()
{
    tb_clear();

    for (auto &item : items_) {
        print_item(item);
        y_++;
    }

    // print_scrollbar down here
    mvprintw(0, tb_height() - 2, fmt::format("The menu contains {} items.", item_count()));

    y_ = 0;
    tb_present();
}

void menu::print_item(const item &t)
{
    int attrs = y_ == selected_item_ ? 0 | TB_REVERSE : 0;

    int x = 0;
    for (char ch : t.nonexacts.title) {
        tb_change_cell(x++, y_, ch, attrs, 0);
    }
}

void menu::mvprintw(int x, int y, string str)
{
    for (char ch : str) {
        tb_change_cell(x++, y, ch, 0, 0);
    }
}

void menu::move(direction dir)
{
    bool at_first_item = selected_item_ == 0,
         at_last_item  = selected_item_ == static_cast<int>(item_count() - 1);

    switch (dir) {
        case up:
            if (at_first_item) return;
            selected_item_--;
            break;
        case down:
            if (at_last_item) return;
            selected_item_++;
            break;
    }

    update();
}

} /* ns bookwyrm */
