#include <cassert>
#include <clocale>

#include "curses_wrap.hpp"
#include "screens/base.hpp"

namespace bookwyrm::tui::screen {

    int base::screen_count_ = 0;

    base::base(int pad_top, int pad_bot, int pad_left, int pad_right)
        : padding_top_(pad_top), padding_bot_(pad_bot), padding_left_(pad_left), padding_right_(pad_right)
    {
        if (screen_count_++ > 0)
            return;
        curses::init();
    }

    base::~base()
    {
        if (--screen_count_ == 0)
            curses::terminate();
        assert(screen_count_ >= 0);
    }

    int base::get_width() const { return curses::get_width() - padding_left_ - padding_right_; }

    int base::get_height() const { return curses::get_height() - padding_top_ - padding_bot_; }

    void base::print(int x, int y, const std::string &str, const attribute attrs, const colour clr)
    {
        x += padding_left_;
        y += padding_top_;

        /* Is the cell owned by the screen? */
        if (!(x <= get_width()) || !(y <= get_height()))
            return;

        curses::mvprint(x, y, str, attrs, clr);
    }

    int base::printlim(int x, int y, const std::string &str, const size_t space, const attribute attrs, const colour clr)
    {
        curses::mvprintn(x, y, str, static_cast<int>(space), attrs, clr);

        int truncd = 0;
        if (str.length() > space) {
            /* The whole string did not fit; indicate this to the user. */

            auto ch = str.cbegin() + static_cast<long int>(space) - 1;
            size_t whitespace = 0;
            while (std::isspace(*(--ch))) {
                ++whitespace;
            }

            truncd = static_cast<int>(str.length() - space + whitespace);
            getyx(stdscr, y, x);
            curses::mvprint(x - static_cast<int>(whitespace) - 1, y, "~", attrs, clr);
        }

        return truncd;
    }

    bool base::action(const int ch)
    {
        const auto move_halfpage = [this](move_direction dir) {
            for (int i = 0; i < get_height() / 2; i++)
                move(dir);
        };

        switch (ch) {
        case 'j':
        case key::arrow_down:
            move(down);
            return true;
        case 'k':
        case key::arrow_up:
            move(up);
            return true;
        case 'g':
            move(top);
            return true;
        case 'G':
            move(bot);
            return true;
        case 'd':
            move_halfpage(down);
            return true;
        case 'u':
            move_halfpage(up);
            return true;
        case key::space:
            toggle_action();
            return true;
        }

        return false;
    }

} // namespace bookwyrm::tui::screen
