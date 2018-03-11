#include <cassert>

#include <fmt/format.h>

#include "screens/base.hpp"

namespace screen {

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

size_t base::get_width() const
{
    int x, y;
    getmaxyx(stdscr, y, x);
    (void)y;
    return x - padding_left_ - padding_right_;
}

size_t base::get_height() const
{
    int x, y;
    getmaxyx(stdscr, y, x);
    (void)x;
    return y - padding_top_ - padding_bot_;
}

void base::change_cell(int x, int y, const uint32_t ch, const colour fg, const colour bg)
{
    x += padding_left_;
    y += padding_top_;

    const bool valid_x = x >= padding_left_ && x <= tb_width() - padding_right_ - 1,
               valid_y = y >= padding_top_ && y <= tb_height() - padding_bot_ - 1;

    if (!valid_x || !valid_y)
        return;

    // TODO: handle colours.
    (void)fg;
    (void)bg;
    mvaddch(y, x, ch);
}

void base::init_tui()
{
    if (screen_count_++ > 0) return;

    /*
     * If this fails, an error is printed to standard output and exit() is called.
     * Can it be done in a better way?
     */
    initscr();

    cbreak();                 // disable line buffering
    keypad(stdscr, true);     // interpret function key escape sequences for us
    noecho();                 // don't echo input to screen
    curs_set(0);              // hide the cursor

    if (has_color()) {
        start_color();        // enable colour support
        use_default_colors(); // set colour index -1 as whatever colour the used terminal background is
    } else {
        // TODO: log warning: colours are not supported.
    }
}

/*
 * This implementation first prints out as much as it cans and then backtracks,
 * wasting oh-so-precious CPU-cycles.
 *
 * TODO: rewrite this to only print as much as it has to.
 */
int base::wprintlim(size_t x, const int y, const string_view &str, const size_t space, const colour attrs)
{
    (void)attrs;

    const size_t limit = x + space - 1;
    for (auto ch = str.cbegin(); ch < str.cend(); ch++) {
        if (x == limit && str.length() > space) {
            /* We can't fit the rest of the string. */

            /* Don't print the substring's trailing whitespace. */
            int whitespace = 0;
            while (std::isspace(*(--ch))) {
                x--;
                whitespace++;
            }

            // TODO: handle colours.
            mvaddch(y, x, '~');
            return str.length() - space + whitespace;
        }

        // TODO: handle colours.
        mvaddch(y, x++, *ch);
    }

    return 0;
}

void base::wprint(int x, const int y, const string_view &str, const colour attrs)
{
    (void)attrs;

    for (const uint32_t &ch : str) {
        // TODO: handle colours.
        mvaddch(y, x++, ch);
    }
}

bool base::action(const key &key, const uint32_t &ch)
{
    const auto move_halfpage = [this] (move_direction dir) {
        for (size_t i = 0; i < get_height() / 2; i++)
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
    }

    return false;
}

/* ns screen */
}
