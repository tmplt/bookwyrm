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

#pragma once

#include <termbox.h>

enum class __key : uint16_t {
    f1               = TB_KEY_F1,
    f2               = TB_KEY_F2,
    f3               = TB_KEY_F3,
    f4               = TB_KEY_F4,
    f5               = TB_KEY_F5,
    f6               = TB_KEY_F6,
    f7               = TB_KEY_F7,
    f8               = TB_KEY_F8,
    f9               = TB_KEY_F9,
    f10              = TB_KEY_F10,
    f11              = TB_KEY_F11,
    f12              = TB_KEY_F12,
    insert           = TB_KEY_INSERT,
    del              = TB_KEY_DELETE,
    home             = TB_KEY_HOME,
    end              = TB_KEY_END,
    pgup             = TB_KEY_PGUP,
    pgdn             = TB_KEY_PGDN,
    arrow_up         = TB_KEY_ARROW_UP,
    arrow_down       = TB_KEY_ARROW_DOWN,
    arrow_left       = TB_KEY_ARROW_LEFT,
    arrow_right      = TB_KEY_ARROW_RIGHT,
    mouse_left       = TB_KEY_MOUSE_LEFT,
    mouse_right      = TB_KEY_MOUSE_RIGHT,
    mouse_middle     = TB_KEY_MOUSE_MIDDLE,
    mouse_release    = TB_KEY_MOUSE_RELEASE,
    mouse_wheel_up   = TB_KEY_MOUSE_WHEEL_UP,
    mouse_wheel_down = TB_KEY_MOUSE_WHEEL_DOWN,

    /* Accordin to termbox.h, these may not be portable. */
    escape = TB_KEY_ESC,
    tab    = TB_KEY_TAB,
    enter  = TB_KEY_ENTER,
    ctrl_l = TB_KEY_CTRL_L,
    ctrl_d = TB_KEY_CTRL_D,
    ctrl_u = TB_KEY_CTRL_U,
    space  = TB_KEY_SPACE
};

enum class __type : uint8_t {
    key_press   = TB_EVENT_KEY,
    resize      = TB_EVENT_RESIZE,
    mouse_press = TB_EVENT_MOUSE
};

namespace keys {

struct event {
    __type type;
    __key key;
    uint32_t ch;
    int32_t w;
    int32_t h;
    int32_t x;
    int32_t y;
};

/*
 * Abstraction of termbox's tb_poll_event.
 * Updates the passed keys::event struct with the data given
 * by tb_event_poll.
 */
bool poll_event(event &ev);

/* ns keys */
}

using key  = __key;
using type = __type;
