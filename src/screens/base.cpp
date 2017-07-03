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

#include "screens/base.hpp"

namespace bookwyrm {

bool screen_base::termbox_started_ = false;

screen_base::screen_base(int pad_top, int pad_bot, int pad_left, int pad_right)
    : padding_top_(pad_top), padding_bot_(pad_bot),
    padding_left_(pad_left), padding_right_(pad_right)
{
    init_tui();
}

screen_base::~screen_base()
{
    if (termbox_started_) tb_shutdown();
}

void screen_base::init_tui()
{
    if (termbox_started_) return;

    int code = tb_init();
    if (code < 0) {
        string err = "termbox init failed with code: " + code;
        throw component_error(err.data());
    }

    tb_set_cursor(TB_HIDE_CURSOR, TB_HIDE_CURSOR);
    tb_clear();
    termbox_started_ = true;
}

int screen_base::mvprintwlim(size_t x, const int y, const string_view &str, const size_t space, const uint16_t attrs)
{
    const size_t limit = x + space;
    for (const uint32_t &ch : str) {
        if (x == limit - 1 && str.length() > space) {
            tb_change_cell(x, y, '~', attrs, 0);
            return str.length() - space;
        }

        tb_change_cell(x++, y, ch, attrs, 0);
    }

    return 0;
}

void screen_base::mvprintw(int x, const int y, const string_view &str, const uint16_t attrs)
{
    for (const uint32_t &ch : str)
        tb_change_cell(x++, y, ch, attrs, 0);
}

void screen_base::mvprintwl(int x, const int y, const string_view &str, const uint16_t attrs)
{
    for (int i = 0; i < x; i++)
        tb_change_cell(i, y, ' ', attrs, 0);

    mvprintw(x, y, str, attrs);

    for (int i = x + str.length(); i < tb_width(); i++)
        tb_change_cell(i, y, ' ', attrs, 0);
}

/* ns bookwyrm */
}
