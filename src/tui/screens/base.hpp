#pragma once

#include <cmath>
#include <functional>
#include <ncurses.h>

#include "colours.hpp"
#include "curses_wrap.hpp"
#include "keys.hpp"
#include "runes.hpp"

namespace bookwyrm::tui::screen {

    constexpr static int default_padding_top = 0, default_padding_bot = 3, default_padding_left = 0,
                         default_padding_right = 0;

    namespace scroll {
        enum { not_applicable = -1 };
    }

    struct paddings {
        paddings(int top, int bot, int left, int right) : top(top), bot(bot), left(left), right(right) {}

        paddings to_absolute() const;

        int top;
        int bot;
        int left;
        int right;
    };

    class base {
    public:
        enum move_direction { top, up, down, bot };

        virtual void paint() = 0;

        /* Update `WINDOW` dimensions */
        virtual void on_resize();

        /* Manage the screen. Return true if an action was performed. */
        virtual bool action(const int ch);

        /* Toggle something on the screen, if anything. */
        virtual void toggle_action();

        /* Move around in/with the screen. */
        virtual void move(move_direction dir);

        /* When this screen is focused, what should we print in the footer? */
        virtual std::string footer_info() const;

        /* How do we control the screen? */
        virtual std::string controls_legacy() const;

        /*
         * How many percent have we scrolled?
         * Returns -1 if we can't scroll yet
         */
        virtual int scrollpercent() const;

        virtual ~base();

    protected:
        explicit base(int pad_top, int pad_bot, int pad_left, int pad_right);

        void refresh();
        void erase();

        /* Like the one in the curses namespace, but for a screens dedicated size
         * instead. */
        int get_width() const;
        int get_height() const;

        /* Validate (x, y) and then print the given string from (x, y) along the
         * x-axis. */
        void print(
            int x, int y, const std::string &str, const attribute attrs = attribute::none, const colour clr = colour::none);
        inline void print(int x, int y, const std::string &str, const colour clr) { print(x, y, str, attribute::none, clr); }

        /*
         * Print a string starting from (x, y) along the x-axis within the space
         * given. If the string is longer than the space it should fit in, the string
         * is truncated with a '~' at its last non-whitespace character. Returns the
         * count of truncated characters, counting from the end of the string.
         */
        int printlim(int x,
                     int y,
                     const std::string &str,
                     const size_t space,
                     const attribute attrs = attribute::none,
                     const colour clr = colour::none);

        void print_right_align(int y, std::string &&str, const colour attrs = colour::none);

        void printcont(int x, const int y, const std::string str, const colour attrs);

        /* Returns the ratio of a int b in percentage. Used for scroll percentage. */
        static inline int ratio(double a, double b) { return static_cast<int>(std::round(100 * (a / b))); }

        const paddings pads_;

    private:
        static int screen_count_;
        WINDOW *window_;
    };

} // namespace bookwyrm::tui::screen
