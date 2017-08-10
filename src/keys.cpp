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

#include <utility>

#include "keys.hpp"

static tb_event tb_ev;

namespace keys {

bool poll_event(event &ev)
{
    if (!tb_poll_event(&tb_ev))
        return false;

    ev.type = type(std::move(tb_ev.type));
    ev.key  = key(std::move(tb_ev.key));
    ev.ch   = std::move(tb_ev.ch);

    ev.w    = std::move(tb_ev.w);
    ev.h    = std::move(tb_ev.h);
    ev.x    = std::move(tb_ev.x);
    ev.y    = std::move(tb_ev.y);

    return true;
}

/* ns keys */
}
