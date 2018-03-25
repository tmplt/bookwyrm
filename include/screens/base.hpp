#pragma once

#include <ncurses.h>

#include "common.hpp"
#include "colours.hpp"
#include "keys.hpp"
#include "runes.hpp"

namespace bookwyrm::screen {

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

    virtual void paint() = 0;

    /* What should be done when the window resizes? */
    virtual void on_resize() { };

    /* Manage the screen. Return true if an action was performed. */
    virtual bool action(const int ch);

    /* Toggle something on the screen, if anything. */
    virtual void toggle_action() { };

    /* Move around in/with the screen. */
    virtual void move(move_direction dir) = 0;

    /* When this screen is focused, what should we print in the footer? */
    virtual string footer_info() const = 0;

    /* How do we control the screen? */
    virtual string controls_legacy() const = 0;

    /*
     * How many percent have we scrolled?
     * Returns -1 if we can't scroll yet
     */
    virtual int scrollpercent() const = 0;

protected:
    explicit base(int pad_top, int pad_bot, int pad_left, int pad_right);
    ~base();

    int get_width() const;
    int get_height() const;

    /*
     * After asserting that the screen owns the cell, change it with the given parameters.
     * Only the first character from the given string is printed.
     */
    void change_cell(int x, int y, const string &str, const colour clr, const attribute attrs = attribute::none);
    void change_cell(int x, int y, const string &str, const attribute attrs = attribute::none)
    {
        change_cell(x, y, str, colour::none, attrs);
    }

    /*
     * Print a string starting from (x, y) along the x-axis within the space given.
     * If the string is longer than the space it should fit in, the string is truncated with a '~' at its last non-whitespace character.
     * Returns the count of truncated characters, counting from the end of the string.
     */
    int wprintlim(int x, int y, const string &str, const size_t space, const colour clr, const attribute attrs = attribute::none);
    int wprintlim(int x, int y, const string &str, const size_t space, const attribute attrs = attribute::none)
    {
        return wprintlim(x, y, str, space, colour::none, attrs);
    }

    /* Same as above, but don't truncate. */
    void wprint(int x, const int y, const string &str, const colour clr = colour::none, const attribute attrs = attribute::none);
    void wprint(int x, const int y, const string &str, const attribute attr)
    {
        wprint(x, y, str, colour::white, attr);
    }

    /* How much space do we leave for bars? */
    int padding_top_, padding_bot_,
        padding_left_, padding_right_;

private:
    static int screen_count_;
};

/* ns screen */
}
