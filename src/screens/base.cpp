#include <cassert>
#include <clocale>

#include <fmt/format.h>

#include "screens/base.hpp"

namespace bookwyrm::screen {

int base::screen_count_ = 0;

base::base(int pad_top, int pad_bot, int pad_left, int pad_right)
    : padding_top_(pad_top), padding_bot_(pad_bot),
    padding_left_(pad_left), padding_right_(pad_right)
{
    init_tui();
}

base::~base()
{
    if (--screen_count_ == 0) endwin();
    assert(screen_count_ >= 0);
}

int base::get_width() const
{
    int x, y;
    getmaxyx(stdscr, y, x);
    (void)y;
    return x - padding_left_ - padding_right_;
}

int base::get_height() const
{
    int x, y;
    getmaxyx(stdscr, y, x);
    (void)x;
    return y - padding_top_ - padding_bot_;
}

void base::change_cell(int x, int y, const string &str, const colour clr, const attribute attrs)
{
    x += padding_left_;
    y += padding_top_;

    /* Is the cell owned by the screen? */
    if (!(x <= get_width()) || !(y <= get_height()))
        return;

    attron(clr | attrs);
    // TODO: only print a single character (keep in mind mb chars)
    mvaddstr(y, x, str.c_str());
    attroff(clr | attrs);
}

void base::change_cell(int x, int y, const uint32_t ch, const colour clr, const attribute attrs)
{
    x += padding_left_;
    y += padding_top_;

    const int width = get_width(),
              height = get_height();

    const bool valid_x = x >= padding_left_ && x <= width - padding_right_ - 1,
               valid_y = y >= padding_top_ && y <= height - padding_bot_ - 1;

    if (!valid_x || !valid_y)
        return;

    attron(clr | attrs);
    mvaddch(y, x, ch);
    attroff(clr | attrs);
}


void base::init_tui()
{
    if (screen_count_++ > 0) return;

    std::setlocale(LC_ALL, "");

    /*
     * If this fails, an error is printed to standard output and exit() is called.
     * Can it be done in a better way?
     */
    initscr();

    cbreak();                 // disable line buffering
    keypad(stdscr, true);     // interpret function key escape sequences for us
    noecho();                 // don't echo input to screen
    curs_set(0);              // hide the cursor

    if (has_colors()) {
        start_color();        // enable colour support
        use_default_colors(); // set colour index -1 as whatever colour the used terminal background is

        init_pair(1, COLOR_BLACK,   -1);
        init_pair(2, COLOR_RED,     -1);
        init_pair(3, COLOR_GREEN,   -1);
        init_pair(4, COLOR_YELLOW,  -1);
        init_pair(5, COLOR_BLUE,    -1);
        init_pair(6, COLOR_MAGENTA, -1);
        init_pair(7, COLOR_CYAN,    -1);
        init_pair(8, COLOR_WHITE,   -1);
    } else {
        // TODO: log warning: colours are not supported.
    }
}

int base::wprintlim(int x, int y, const string &str, const size_t space, const colour clr, const attribute attrs)
{
    attron(clr | attrs);
    mvaddnstr(y, x, str.c_str(), space);

    int truncd = 0;
    if (str.length() > space) {
        /* The whole string did not fit; indicate this to the user. */

        auto ch = str.cbegin() + space - 1;
        int whitespace = 0;
        while (std::isspace(*(--ch))) {
            ++whitespace;
        }

        truncd = str.length() - space + whitespace;
        getyx(stdscr, y, x);
        mvaddch(y, x - whitespace - 1, '~');
    }

    attroff(clr | attrs);
    return truncd;
}

void base::wprint(int x, const int y, const string_view &str, const colour attrs)
{
    (void)attrs;

    for (const uint32_t &ch : str) {
        // TODO: handle colours.
        mvaddch(y, x++, ch);
    }
}

void base::wprint(int x, const int y, const string_view &str, const colour clr, const attribute attrs)
{

    attron(clr | attrs);

    for (const uint32_t &ch : str)
        mvaddch(y, x++, ch);

    attroff(clr | attrs);
}

bool base::action(const key &key, const uint32_t &ch)
{
    const auto move_halfpage = [this] (move_direction dir) {
        for (int i = 0; i < get_height() / 2; i++)
            move(dir);
    };

    switch (key) {
        case key::arrow_down:
            move(down);
            return true;
        case key::arrow_up:
            move(up);
            return true;
        case key::space:
            toggle_action();
            return true;
        case key::ctrl_d:
            move_halfpage(down);
            return true;
        case key::ctrl_u:
            move_halfpage(up);
            return true;
        default:
            break;
    }

    switch (ch) {
        case 'j':
            move(down);
            return true;
        case 'k':
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
        case 's':
            toggle_action();
            return true;
    }

    return false;
}

/* ns screen */
}
