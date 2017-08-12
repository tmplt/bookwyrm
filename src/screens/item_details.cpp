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

#include "screens/item_details.hpp"

namespace screen {

item_details::item_details(const bookwyrm::item &item, int padding_top)
    : base(padding_top, default_padding_bot, 0, 0), item_(item)
{

}

bool item_details::action(const key &key, const uint32_t &ch)
{
    (void)key;
    (void)ch;

    return false;
}

void item_details::update()
{
    print_borders();
    print_text();
}

void item_details::on_resize()
{
    update();
}

string item_details::footer_info() const
{
    return fmt::format("DEBUG: padding top: {}, height: {}", padding_top_, get_height());
}

void item_details::print_borders()
{
    const auto print_line = [this](int y) {
        for (int x = 0; x <= get_width(); x++)
            change_cell(x, y, ascii::em_dash);
    };

    print_line(0);
    print_line(get_height() - 1);
}

void item_details::print_text()
{
    const string output = "Test output";

    /* Find the center of the screen. */
    const int x = get_width() / 2 - output.length() / 2,
              y = get_height() / 2;

    mvprintw(x, y, output, colour::blue | attribute::bold | attribute::underline);
}

/* ns screen */
}
