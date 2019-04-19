#include <cassert>
#include <clocale>

#include "curses_wrap.hpp"
#include "screens/base.hpp"

namespace bookwyrm::tui::screen {

    int base::screen_count_ = 0;

    /*
     * Translate padding values from relative to absolute, with the current
     * terminal dimensions in mind.
     */
    paddings paddings::to_absolute() const
    {
        auto absolute_padding = [](auto get_fullsize, int p) -> int { return (p < 0 ? get_fullsize(stdscr) : 0) + p; };

        paddings abs(absolute_padding(curses::get_height, top),
                     absolute_padding(curses::get_height, bot),
                     absolute_padding(curses::get_width, left),
                     absolute_padding(curses::get_width, right));

        return std::move(abs);
    }

    base::base(int pad_top, int pad_bot, int pad_left, int pad_right) : pads_(pad_top, pad_bot, pad_left, pad_right)
    {
        if (screen_count_++ == 0) {
            /* TODO: notify user if colours are not supported and. c.f. NO_COLOR. */
            curses::init();
        }

        auto abs = pads_.to_absolute();
        window_ =
            newwin(curses::get_height() - abs.top - abs.bot, curses::get_width() - abs.right - abs.left, abs.top, abs.left);

        curses::keypad(window_, true);  // interpret function key escape sequences for us
        curses::nodelay(window_, true); // make wgetch(3) a non-blocking call
    }

    base::~base()
    {
        if (--screen_count_ == 0)
            curses::terminate();
        assert(screen_count_ >= 0);

        curses::delwin(window_);
    }

    int base::get_width() const { return curses::get_width(window_); }

    int base::get_height() const { return curses::get_height(window_); }

    void base::print(int x, int y, const std::string &str, const attribute attrs, const colour clr)
    {
        curses::mvprint(window_, x, y, str, attrs, clr);
    }

    int base::printlim(int x, int y, const std::string &str, const size_t space, const attribute attrs, const colour clr)
    {
        curses::mvprintn(window_, x, y, str, static_cast<int>(space), attrs, clr);

        int truncd = 0;
        if (str.length() > space) {
            /* The whole string did not fit; indicate this to the user. */

            /* From the substing written in the space, find the number of whitespaces from the end of the string to the first
             * multi-character word. Uses look ahead; can STL be used? */
            auto ch = str.cbegin() + space - 1;
            size_t whitespace = 0;
            while (std::isspace(*(--ch)))
                ++whitespace;

            truncd = str.length() - space + whitespace;
            curses::mvprint(window_, x + str.length() - truncd - 1, y, "~", attrs, clr);
        }

        return truncd;
    }

    void base::print_right_align(int y, std::string &&str, const colour attrs)
    {
        print(curses::get_width(window_) - str.length(), y, str, attrs);
    }

    void base::printcont(int x, const int y, const std::string str, const colour attrs)
    {
        curses::mvprint(window_, x, y, str, attribute::none, attrs);

        for (int i = x + str.length(); i < curses::get_width(); i++)
            curses::mvprint(window_, i, y, " ", attribute::none, attrs);
    }

    void base::on_resize()
    {
        auto abs = pads_.to_absolute();
        curses::wresize(window_, curses::get_height() - abs.top - abs.bot, curses::get_width() - abs.left - abs.right);
        curses::mvwin(window_, abs.top, abs.bot);
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

    void base::toggle_action() {}

    void base::move(move_direction) {}

    std::string base::footer_info() const { return ""; }

    std::string base::controls_legacy() const { return ""; }

    int base::scrollpercent() const { return -1; }

    void base::refresh() { curses::wnoutrefresh(window_); }

    void base::erase() { curses::werase(window_); }

    int base::getkey() { return curses::getkey(window_); }

} // namespace bookwyrm::tui::screen
