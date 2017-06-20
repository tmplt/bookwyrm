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
#include <iostream>
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

    std::cout << "selected items:\n";
    for (auto idx : marked_items_)
        std::cout << idx << ' ';

    std::cout << '\n';
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

    /*
     * For some odd reason, we must press ESC twice
     * to quit here.
     * TODO: figure out why.
     */
    bool quit = false;
    struct tb_event ev;
    while (tb_poll_event(&ev) && !quit) {
        if (ev.type == TB_EVENT_RESIZE) {
            resize();
        } else if (ev.type == TB_EVENT_KEY) {
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
                case TB_KEY_SPACE:
                    toggle_select();
                    break;
            }

            switch (ev.ch) {
                case 'j':
                    move(down);
                    break;
                case 'k':
                    move(up);
                    break;
                case 'g':
                    move(top);
                    break;
                case 'G':
                    move(bot);
                    break;
            }
        }
    }
}

void menu::update()
{
    tb_clear();

    for (size_t i = scroll_offset_; i < item_count(); i++) {
        if (y_ == menu_capacity()) break;
        print_item(items_[i]);
        y_++;
    }

    print_scrollbar();

    if (menu_at_bot()) mvprintw(0, tb_height() - 1, "bot");
    if (menu_at_top()) mvprintw(0, tb_height() - 1, "top");

    mvprintw(0, tb_height() - 2, fmt::format("The menu contains {} items.", item_count()));
    mvprintw(5, tb_height() - 1, fmt::format("selected_item_ = {}, scroll_offset_ = {}",
                selected_item_, scroll_offset_));

    y_ = 0;
    tb_present();
}

void menu::print_item(const item &t)
{
    bool on_selected_item = (y_ + scroll_offset_ == selected_item_);
    size_t offset_idx = y_ + scroll_offset_;

    /*
     * Imitate an Ncurses menu, denote the selected item with a '-'
     * and by reversing fg and bg on the entry.
     * Leave x = 0 to the indicator.
     */
    if (on_selected_item)
        tb_change_cell(0, y_, '-', 0, 0);
    else if (is_marked(offset_idx))
        tb_change_cell(0, y_, ' ', TB_REVERSE, 0);

    uint16_t attrs = (on_selected_item || is_marked(offset_idx))
        ? TB_REVERSE : 0;

    int x = 1;
    for (uint32_t ch : t.nonexacts.title) {
        tb_change_cell(x++, y_, ch, attrs, 0);
    }
}

void menu::mvprintw(int x, int y, string str)
{
    for (uint32_t ch : str) {
        tb_change_cell(x++, y, ch, 0, 0);
    }
}

void menu::move(move_direction dir)
{
    bool at_first_item = selected_item_ == 0,
         at_last_item  = selected_item_ == static_cast<int>(item_count() - 1);

    switch (dir) {
        case up:
            if (at_first_item) return;
            if (menu_at_top()) scroll_offset_--;
            selected_item_--;
            break;
        case down:
            if (at_last_item) return;
            if (menu_at_bot()) scroll_offset_++;
            selected_item_++;
            break;
        case top:
            scroll_offset_ = selected_item_ = 0;
            break;
        case bot:
            selected_item_ = item_count() - 1;
            scroll_offset_ = selected_item_ - menu_capacity() + 1;
            break;
    }

    update();
}

void menu::toggle_select()
{
    if (is_marked(selected_item_))
        unmark_item(selected_item_);
    else
        mark_item(selected_item_);

    update();
}

void menu::resize()
{
    /*
     * When the window is resized from the lower left/right
     * corner, the currently selected item may escape the
     * menu, so we lock it here.
     */
    if (menu_at_bot()) selected_item_--;
    update();
}

void menu::print_scrollbar()
{
    /*
     * This isn't something we can plug-and-play later,
     * and it might not scale perfectly, but it gives us
     * something to work from.
     */

    /* Nice runes. */
    const uint32_t bg = 0x2592, // '▒'
                   fg = 0x2588; // '█'

    /*
     * Find the height of the scrollbar.
     * The more entries, the smaller is gets.
     * (not less than 2, though)
     */
    const size_t height = std::max((tb_height() - 2) / item_count(), 3ul) - 1;

    /* Find out where to print the bar. */
    const size_t start = selected_item_ * (tb_height() - 4) / item_count() + 1;

    /* First print the scrollbar's background. */
    for (int y = 1; y <= tb_height() - 2; y++) {
        tb_change_cell(tb_width() - 1, y, bg, 0, 0);
    }

    /* Then we print the bar. */
    for (size_t y = start; y <= start + height; y++) {
        tb_change_cell(tb_width() - 1, y, fg, 0, 0);
    }
}

} /* ns bookwyrm */
