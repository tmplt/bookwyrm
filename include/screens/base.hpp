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
#include "colours.hpp"
#include "keys.hpp"
#include "ascii.hpp"

namespace screen {

constexpr static int default_padding_top   = 0,
                     default_padding_bot   = 3,
                     default_padding_left  = 0,
                     default_padding_right = 0;

namespace scroll {
enum { not_applicable = -1 };
}

/*
 * A base screen class holding most (if not all) of the
 * implementation of functions using whatever library for
 * printing the actual characters on screen.
 */
class base {
public:

    enum move_direction { top, up, down, bot };

    /* Paint the screen. */
    virtual void update() = 0;

    /* What should be done when the window resizes? */
    virtual void on_resize() = 0;

    /* Manage the screen. Return true if an action was performed. */
    virtual bool action(const key &key, const uint32_t &ch);

    /* Toggle something on the screen, if anything. */
    virtual void toggle_action() { };

    /* Move around in/with the screen. */
    virtual void move(move_direction dir) = 0;

    /* When this screen is focused, what should we print in the footer? */
    virtual string footer_info() const = 0;

    /* How do we control the screen? */
    virtual string footer_controls() const = 0;

    /*
     * How many percent have we scrolled?
     * Returns -1 if we can't scroll yet
     */
    virtual int scrollperc() const = 0;

protected:
    explicit base(int pad_top, int pad_bot, int pad_left, int pad_right);
    ~base();

    size_t get_width() const;
    size_t get_height() const;

    /*
     * After asserting that the screen owns the cell,
     * change it with the given parameters.
     */
    void change_cell(int x, int y, const uint32_t ch, const colour fg = colour::none, const colour bg = colour::none);

    void change_cell(int x, int y, const uint32_t ch, const attribute attr)
    {
        change_cell(x, y, ch, colour::none | attr);
    }

    void change_cell(int x, int y, const ascii ch, const attribute attr)
    {
        change_cell(x, y, static_cast<ascii_t>(ch), colour::none | attr);
    }

    void change_cell(int x, int y, const ascii ch)
    {
        change_cell(x, y, static_cast<ascii_t>(ch));
    }

    /*
     * Akin to Ncurses mvprintw(), but:
     * print a string starting from (x, y) along the x-axis. The space
     * argument denotes how much of the string is printed. If the string
     * doesn't fit, the string is truncated with '~'.
     *
     * Returns the count of truncated characters, counter from the end of
     * the string.
     */
    int mvprintwlim(size_t x, const int y, const string_view &str, const size_t space, const colour attrs = colour::white);
    int mvprintwlim(size_t x, const int y, const string_view &str, const size_t space, const attribute attr)
    {
        return mvprintwlim(x, y, str, space, colour::white | attr);
    }

    /* Same as above, but don't truncate. */
    void mvprintw(int x, const int y, const string_view &str, const colour attrs = colour::white);
    void mvprintw(int x, const int y, const string_view &str, const attribute attr)
    {
        mvprintw(x, y, str, colour::white | attr);
    }

    /* How much space do we leave for bars? */
    int padding_top_, padding_bot_,
        padding_left_, padding_right_;

private:
    static int screen_count_;
    static void init_tui();
};

/* ns screen */
}
