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

#include <cassert>

#include "screens/base.hpp"

namespace screen {

int base::screen_count_ = 0;

base::base(int pad_top, int pad_bot, int pad_left, int pad_right)
    : padding_top_(pad_top), padding_bot_(pad_bot),
    padding_left_(pad_left), padding_right_(pad_right)
{
    init_tui();
}

base::~base()
{
    if (--screen_count_ == 0) tb_shutdown();
    assert(screen_count_ >= 0);
}

void base::change_cell(const int x, const int y, const uint32_t ch, const colour fg)
{
    const bool valid_x = x >= padding_left_ && x < get_width() - padding_right_,
               valid_y = y <= get_height() - padding_bot_ - 1 && y >= padding_top_;
    if (!valid_x || !valid_y)
        return;

    tb_change_cell(x, y, ch, static_cast<colour_t>(fg), 0);
}

void base::init_tui()
{
    if (screen_count_++ > 0) return;

    int code = tb_init();
    if (code < 0) {
        string err = "termbox init failed with code: " + code;
        throw component_error(err.data());
    }

    tb_select_output_mode(TB_OUTPUT_NORMAL);
    tb_set_cursor(TB_HIDE_CURSOR, TB_HIDE_CURSOR);
    tb_clear();
}

int base::mvprintwlim(size_t x, const int y, const string_view &str, const size_t space, const colour attrs)
{
    const size_t limit = x + space;
    for (const uint32_t &ch : str) {
        if (x == limit - 1 && str.length() > space) {
            /* We can't fit the rest of the string. */
            change_cell(x, y, '~', attrs);
            return str.length() - space;
        }

        change_cell(x++, y, ch, attrs);
    }

    return 0;
}

void base::mvprintw(int x, const int y, const string_view &str, const colour attrs)
{
    for (const uint32_t &ch : str)
        change_cell(x++, y, ch, attrs);
}

/* ns screen */
}
