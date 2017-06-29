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

#include "errors.hpp"
#include "python.hpp"
#include "screens/multiselect_menu.hpp"

namespace py = pybind11;

namespace bookwyrm {

multiselect_menu::multiselect_menu(searcher &s)
    : screen_base(1, 3, 0, 1),
    selected_item_(0), scroll_offset_(0),
    items_(s.results())
{
    /*
     * These wanted widths works fine for now,
     * but we might want some way to utilize whatever
     * horizontal space is left.
     */
    columns_ = {
        {"title",      .30},
        {"year",       4  },
        {"serie",      .15},
        {"authors",    .20},
        {"publisher",  .15},
        {"format",     6  },
    };

    update_column_widths();

    s.set_menu(this);
    s.async_search();
}

void multiselect_menu::display()
{
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
            on_resize();
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
                case TB_KEY_CTRL_L:
                    update();
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

void multiselect_menu::update()
{
    tb_clear();

    for (size_t col_idx = 0; col_idx < columns_.size(); col_idx++) {
        if (columns_[col_idx].width > tb_width() - 1 - columns_[col_idx].startx) {
            /* We can't fit another column. */
            break;
        }

        print_column(col_idx);
    }

    print_header();
    print_scrollbar();

    if (menu_at_bot()) mvprintw(0, tb_height() - 1, "bot");
    if (menu_at_top()) mvprintw(0, tb_height() - 1, "top");

    mvprintw(0, tb_height() - 2, fmt::format("The menu contains {} items.", item_count()));
    mvprintw(5, tb_height() - 1, fmt::format(
                "selected_item_ = {}, scroll_offset_ = {}, menu_capacity = {}",
                selected_item_, scroll_offset_, menu_capacity()));

    tb_present();
}

void multiselect_menu::move(move_direction dir)
{
    const bool at_first_item = selected_item_ == 0,
               at_last_item  = selected_item_ == (item_count() - 1);

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

void multiselect_menu::toggle_select()
{
    if (is_marked(selected_item_))
        unmark_item(selected_item_);
    else
        mark_item(selected_item_);

    update();
}

void multiselect_menu::update_column_widths()
{
    size_t x = 1;
    for (auto &column : columns_) {
        try {
            column.width = std::get<int>(column.width_w);
        } catch (std::bad_variant_access&) {
            const int width = tb_width() - 1 - padding_right_;
            column.width = width * std::get<double>(column.width_w);
        }

        column.startx = x;
        x += column.width + 3; // We want a 1 char padding on both sides of the seperator.
    }
}

void multiselect_menu::on_resize()
{
    update_column_widths();

    /*
     * When the window is resized from the lower left/right
     * corner, the currently selected item may escape the
     * menu, so we lock it here.
     *
     * TODO: fix this for cases where the cursor has
     * jumped more than one step down under.
     * (check whether it's just below what's valid, and
     * if so, move it to menu_bot).
     */
    if (menu_at_bot()) selected_item_--;
    update();
}

void multiselect_menu::print_scrollbar()
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
     * (not less than 1, though)
     */
    const size_t height = std::max((menu_capacity()) / item_count(), 3ul) - 2;

    /* Find out where to print the bar. */
    const size_t start = selected_item_ * (menu_capacity() - 1) / item_count() + 1;

    /* First print the scrollbar's background. */
    for (size_t y = 1; y <= menu_capacity(); y++) {
        tb_change_cell(tb_width() - 1, y, bg, 0, 0);
    }

    /* Then we print the bar. */
    for (size_t y = start; y <= start + height; y++) {
        tb_change_cell(tb_width() - 1, y, fg, 0, 0);
    }
}

void multiselect_menu::print_header()
{
    /*
     * You might think we should start at x = 0, but that
     * screws up the alignment with the column strings.
     */
    size_t x = 1;
    for (auto &column : columns_) {
        if (column.width > tb_width() - 1 - padding_right_ - x)
            break;

        /* Center the title. */
        mvprintw(x + column.width / 2  - column.title.length() / 2, 0, column.title);
        x += std::max(column.width, column.title.length()) + 1;

        mvprintw(x, 0, "|");
        x += 2;
    }
}

void multiselect_menu::print_column(const size_t col_idx)
{
    const auto &c = columns_[col_idx];

    for (size_t i = scroll_offset_, y = padding_top_; i < item_count() &&
            y <= menu_capacity(); i++, y++) {

        const bool on_selected_item = (y + scroll_offset_ == selected_item_ + padding_top_),
                   marked = is_marked(y + scroll_offset_ - padding_top_);

        /*
         * Print the indicator, indicating which item is
         * currently selected.
         */
        if (on_selected_item && marked)
            tb_change_cell(0, y, '-', TB_REVERSE, 0);
        else if (on_selected_item)
            tb_change_cell(0, y, '-', 0, 0);
        else if (marked)
            tb_change_cell(0, y, ' ', TB_REVERSE, 0);

        const uint16_t attrs = (on_selected_item || marked)
            ? TB_REVERSE : 0;

        /* Print the string, check if it was truncated. */
        const auto &str = items_[i].menu_order(col_idx);
        const int truncd = mvprintwl(c.startx, y, str, c.width, attrs);

        /*
         * Fill the space between the two column strings with inverted spaces.
         * This makes the whole line seem selected instead of only the strings.
         *
         * We start at the end of the string, just after the last character (or the '~'),
         * and write until the end of the column, plus seperator and the padding on the right
         * side of it (e.g. up to and including the first char in the next column.
         */
        for (auto x = c.startx + str.length() - truncd; x <= c.startx + c.width + 4; x++) {
            tb_change_cell(x, y, ' ', attrs, 0);
        }
    }
}

} /* ns bookwyrm */
