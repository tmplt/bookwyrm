#pragma once

#include <cmath>
#include <ncurses.h>

#include "colours.hpp"
#include "keys.hpp"
#include "runes.hpp"

namespace bookwyrm::tui::screen {

constexpr static int default_padding_top   = 0,
                     default_padding_bot   = 3,
                     default_padding_left  = 0,
                     default_padding_right = 0;

namespace scroll {
enum { not_applicable = -1 };
}

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
    virtual std::string footer_info() const = 0;

    /* How do we control the screen? */
    virtual std::string controls_legacy() const = 0;

    /*
     * How many percent have we scrolled?
     * Returns -1 if we can't scroll yet
     */
    virtual int scrollpercent() const = 0;

protected:
    explicit base(int pad_top, int pad_bot, int pad_left, int pad_right);
    ~base();

    /* Like the one in the curses namespace, but for a screens dedicated size instead. */
    int get_width() const;
    int get_height() const;

    /* Validate (x, y) and then print the given string from (x, y) along the x-axis. */
    void print(int x, int y, const std::string &str, const attribute attrs = attribute::none, const colour clr = colour::none);
    void print(int x, int y, const std::string &str, const colour clr)
    {
        print(x, y, str, attribute::none, clr);
    }

    /*
     * Print a string starting from (x, y) along the x-axis within the space given.
     * If the string is longer than the space it should fit in, the string is truncated with a '~' at its last non-whitespace character.
     * Returns the count of truncated characters, counting from the end of the string.
     */
    int printlim(int x, int y, const std::string &str, const size_t space, const attribute attrs = attribute::none, const colour clr = colour::none);

    /* Returns the ratio of a int b in percentage. Used for scroll percentage. */
    static inline int ratio(double a, double b)
    {
        return std::round<int>(100 * (a / b));
    }

    /* How much space do we leave for bars? */
    int padding_top_, padding_bot_,
        padding_left_, padding_right_;

private:
    static int screen_count_;
};

/* ns bookwyrm::tui::screen */
}
