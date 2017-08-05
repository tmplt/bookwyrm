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
#include "screens/item_details.hpp"

namespace screen {

multiselect_menu::multiselect_menu(vector<bookwyrm::item> const &items)
    : base(default_padding_top, default_padding_bot, default_padding_left, 0),
    selected_item_(0), scroll_offset_(0),
    items_(items)
{
    /*
     * These wanted widths works fine for now,
     * but we are still not utilizing the full 100%
     * of the width, which we should.
     *
     * TODO: Remedy this.
     */
    columns_ = {
        {"title",     .30},
        {"year",       4 },
        {"serie",     .15},
        {"authors",   .20},
        {"publisher", .15},
        {"format",     6 },
    };

    update_column_widths();
}

bool multiselect_menu::action(const uint16_t &key, const uint32_t &ch)
{
    switch (key) {
        case TB_KEY_ARROW_DOWN:
            move(down);
            return true;
        case TB_KEY_ARROW_UP:
            move(up);
            return true;
        case TB_KEY_SPACE:
            toggle_select();
            return true;
    }

    switch (ch) {
        case 'j':
            move(down);
            return true;
        case 'k':
            move(up);
            return true;
        case 'g':
            move(top);
            return true;
        case 'G':
            move(bot);
            return true;
    }

    return false;
}

void multiselect_menu::update()
{
    for (size_t idx = 0; idx < columns_.size(); idx++) {
        /* Can we fit another column? */
        const size_t allowed_width = get_width() - 1 - columns_[idx].startx;
        if (columns_[idx].width > allowed_width) break;

        print_column(idx);
    }

    print_header();
    print_scrollbar();
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
}

void multiselect_menu::toggle_select()
{
    if (is_marked(selected_item_))
        unmark_item(selected_item_);
    else
        mark_item(selected_item_);
}

void multiselect_menu::update_column_widths()
{
    size_t x = 1;
    for (auto &column : columns_) {
        try {
            column.width = std::get<int>(column.width_w);
        } catch (std::bad_variant_access&) {
            /* It's a ratio, so multiply it with the full width. */
            const int width = get_width() - 1 - padding_right_;
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
}

void multiselect_menu::print_scrollbar()
{
    /* Nice runes. */
    const uint32_t bg = 0x2592, // '▒'
                   fg = 0x2588; // '█'

    /*
     * Find the height of the scrollbar.
     * The more entries, the smaller is gets.
     * (not less than 3 - 2 = 1, though)
     */
    const size_t height = std::max<size_t>(menu_capacity() / item_count(), 3) - 2;

    /* Find out where to print the bar. */
    const size_t start = selected_item_ * (menu_capacity() - 1) / item_count() + virtual_padding_top();

    /* First print the scrollbar's background. */
    for (size_t y = virtual_padding_top(); y <= menu_capacity(); y++)
        change_cell(get_width() - 1, y, bg, 0, 0);

    /* Then we print the bar. */
    for (size_t y = start; y <= start + height; y++)
        change_cell(get_width() - 1, y, fg, 0, 0);
}

void multiselect_menu::print_header()
{
    /*
     * You might think we should start at x = 0, but that
     * screws up the alignment with the column strings.
     */
    size_t x = 1;
    for (auto &column : columns_) {
        /* Center the title. */
        mvprintw(x + column.width / 2  - column.title.length() / 2, 0, column.title);
        x += std::max(column.width, column.title.length());

        /* Padding between the title and the seperator to the left.. */
        x++;

        /* Print the seperator. */
        mvprintw(x++, 0, "|");

        /* ..and to the right. */
        x++;
    }
}

void multiselect_menu::print_footer()
{
    mvprintw(0, get_height() - 2, fmt::format("I have found {} items thus far.", item_count()));
    mvprintwl(0, get_height() - 1, "[ESC]Quit [j/k]Navigation [SPACE]Toggle select", TB_REVERSE | TB_BOLD);
}

void multiselect_menu::print_column(const size_t col_idx)
{
    const auto &c = columns_[col_idx];

    for (size_t i = scroll_offset_, y = virtual_padding_top(); i < item_count() &&
            y <= menu_capacity(); i++, y++) {

        const bool on_selected_item = (y + scroll_offset_ == selected_item_ + virtual_padding_top()),
                   on_marked_item   = is_marked(y + scroll_offset_ - virtual_padding_top());

        /*
         * Print the indicator, indicating which item is
         * currently selected.
         */
        if (on_selected_item && on_marked_item)
            change_cell(0, y, '-', TB_REVERSE, 0);
        else if (on_selected_item)
            change_cell(0, y, '-', 0, 0);
        else if (on_marked_item)
            change_cell(0, y, ' ', TB_REVERSE, 0);

        const uint16_t attrs = (on_selected_item || on_marked_item)
            ? TB_REVERSE : 0;

        /* Print the string, check if it was truncated. */
        const auto &str = items_[i].menu_order(col_idx);
        const int trunc_len = mvprintwlim(c.startx, y, str, c.width, attrs);

        /*
         * Fill the space between the two column strings with inverted spaces.
         * This makes the whole line seem selected instead of only the strings.
         *
         * We start at the end of the string, just after the last character (or the '~'),
         * and write until the end of the column, plus seperator and the padding on the right
         * side of it (e.g. up to and including the first char in the next column, hence the magic).
         */
        const auto string_end = c.startx + str.length() - trunc_len,
                   next_start = c.startx + c.width + 2;
        for (auto x = string_end; x <= next_start; x++)
            change_cell(x, y, ' ', attrs, 0);
    }
}

const std::pair<int, int> multiselect_menu::compress()
{
    const int details_height = menu_capacity() * 0.80;
    padding_bot_ = menu_capacity() * 0.80;

    /*
     * Will the detail menu hide the highlighted item?
     * How much do we need to scroll if we don't want that to happen?
     */
    const int scroll = std::max<int>(selected_item_ - scroll_offset_ - menu_capacity() + 1, 0);
    scroll_offset_ += scroll;

    return {scroll, details_height};
}

void multiselect_menu::decompress(int scroll)
{
    padding_bot_ = default_padding_bot;
    scroll_offset_ -= scroll;
}

} /* ns screen */
