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
    virtual bool action(const key &key, const uint32_t &ch);

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

    void change_cell(int x, int y, const rune::single ch, const attribute attr)
    {
        change_cell(x, y, static_cast<rune_t>(ch), colour::none | attr);
    }

    void change_cell(int x, int y, const rune::single ch)
    {
        change_cell(x, y, static_cast<rune_t>(ch));
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
    int wprintlim(size_t x, const int y, const string_view &str, const size_t space, const colour attrs = colour::white);
    int wprintlim(size_t x, const int y, const string_view &str, const size_t space, const attribute attr)
    {
        return wprintlim(x, y, str, space, colour::white | attr);
    }

    /* Same as above, but don't truncate. */
    void wprint(int x, const int y, const string_view &str, const colour attrs = colour::white);
    void wprint(int x, const int y, const string_view &str, const attribute attr)
    {
        wprint(x, y, str, colour::white | attr);
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
