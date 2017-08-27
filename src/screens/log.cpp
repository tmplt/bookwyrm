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

#include "screens/log.hpp"

namespace screen {

log::log()
    : base(default_padding_top, default_padding_bot, default_padding_left, 0)
{

}

bool log::action(const key &key, const uint32_t &ch)
{
    /* stub */
    (void)key;
    (void)ch;

    return false;
}

void log::update()
{
    /* stub */
}

void log::on_resize()
{
    /* stub */
}

string log::footer_info() const
{
    /* stub */
    return fmt::format("You're in the log now. Entries: {}", entries_.size());
}

int log::scrollperc() const
{
    /* stub */
    return 42;
}

/* ns screen */
}
