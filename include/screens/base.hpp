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

#include "common.hpp"

namespace screen {

constexpr static int default_padding_top  = 1,
                     default_padding_left = 1,
                     default_padding_bot  = 3;

/*
 * A base screen class holding most (if not all) of the
 * implementation of functions using whatever library for
 * printing the actual characters on screen.
 */
class base {
public:
    /* Paint the screen. */
    virtual void update() = 0;

    /* What should be done when the window resizes? */
    virtual void on_resize() = 0;

    /* Handle keybinds. TODO: rename? */
    virtual void action(const uint16_t &key, const uint32_t &ch) = 0;

    /* Returns true if the bookwyrm fits in the current terminal window. */
    static bool bookwyrm_fits()
    {
        /*
         * I planned to use the classical 80x24, but multiselect_menu is
         * in its current form useable in terminals much smaller
         * than that.
         */
        return get_width() >= 50 && get_height() >= 10;
    }

protected:
    explicit base(int pad_top, int pad_bot, int pad_left, int pad_right);
    ~base();

    static int get_width()  { return tb_width(); }
    static int get_height() { return tb_height(); }

    /*
     * Akin to Ncurses mvprintw(), but:
     * print a string starting from (x, y) along the x-axis. The space
     * argument denotes how much of the string is printed. If the string
     * doesn't fit, the string is truncated with '~'.
     *
     * Returns the count of truncated characters, counter from the end of
     * the string.
     */
    static int mvprintwlim(size_t x, const int y, const string_view &str, const size_t space, const uint16_t attrs = 0);

    /* Same as above, but don't truncate. */
    static void mvprintw(int x, const int y, const string_view &str, const uint16_t attrs = 0);

    /*
     * Print passed string starting from (x, y) along the x-axis.
     * All other cells on the same line will be empty (' ') with
     * attrs applied.
     */
    static void mvprintwl(int x, const int y, const string_view &str, const uint16_t attrs = 0);

    /* How much space do we leave for bars? */
    int padding_top_, padding_bot_,
        padding_left_, padding_right_;

private:
    static int screen_count_;
    static void init_tui();
};

/* ns screen */
}
