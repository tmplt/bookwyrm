#pragma once

#include <ncurses.h>

enum key : int {
    // TODO: translate to ncurses
    f1               = KEY_F(1),
    f2               = KEY_F(2),
    f3               = KEY_F(3),
    f4               = KEY_F(4),
    f5               = KEY_F(5),
    f6               = KEY_F(6),
    f7               = KEY_F(7),
    f8               = KEY_F(8),
    f9               = KEY_F(9),
    f10              = KEY_F(10),
    f11              = KEY_F(11),
    f12              = KEY_F(12),
    insert           = KEY_IL,
    del              = KEY_DC,
    home             = KEY_HOME,
    end              = KEY_END,
    pgup             = KEY_PPAGE,
    pgdn             = KEY_NPAGE,
    arrow_up         = KEY_UP,
    arrow_down       = KEY_DOWN,
    arrow_left       = KEY_LEFT,
    arrow_right      = KEY_RIGHT,
    mouse_left       = 0,
    mouse_right      = 0,
    mouse_middle     = 0,
    mouse_release    = 0,
    mouse_wheel_up   = 0,
    mouse_wheel_down = 0,

    escape ,
    tab    = KEY_STAB,
    enter  = KEY_ENTER,
    ctrl_l ,
    ctrl_d ,
    ctrl_u ,
    space ,
    resize = KEY_RESIZE
};

enum class __type : int {
    // TODO: translate to ncurses
    key_press   = 0,
    resize      = KEY_RESIZE,
    mouse_press = 0
};

/* namespace keys { */

/* struct event { */
/*     __type type; */
/*     __key key; */
/*     uint32_t ch; */
/*     int32_t w; */
/*     int32_t h; */
/*     int32_t x; */
/*     int32_t y; */
/* }; */

/*
 * Abstraction of termbox's tb_poll_event.
 * Updates the passed keys::event struct with the data given
 * by tb_event_poll.
 */
/* bool poll_event(event &ev); */

/* ns keys */
/* } */

/* using key  = __key; */
/* using type = __type; */
