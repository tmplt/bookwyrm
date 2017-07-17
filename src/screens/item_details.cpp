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

void item_details::action(const uint16_t &key, const uint32_t &ch)
{
    (void)key;
    (void)ch;
}

void item_details::update()
{

}

void item_details::on_resize()
{

}

/* ns screen */
}
