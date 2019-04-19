#pragma once

#include <ncurses.h>
#include <tuple>
#include "colours.hpp"

namespace bookwyrm::tui::curses {

    inline bool init()
    {
        /*
         * If this fails, an error is printed to standard output and exit() is called.
         * Can it be done in a better way?
         */
        initscr();

        cbreak();    // disable line buffering
        noecho();    // don't echo input to screen
        curs_set(0); // hide the cursor

        if (has_colors()) {
            start_color();        // enable colour support
            use_default_colors(); // set colour index -1 as whatever colour the used
                                  // terminal background is

            init_pair(1, COLOR_BLACK, -1);
            init_pair(2, COLOR_RED, -1);
            init_pair(3, COLOR_GREEN, -1);
            init_pair(4, COLOR_YELLOW, -1);
            init_pair(5, COLOR_BLUE, -1);
            init_pair(6, COLOR_MAGENTA, -1);
            init_pair(7, COLOR_CYAN, -1);
            init_pair(8, COLOR_WHITE, -1);

            return true;
        } else {
            return false;
        }
    }

    inline void terminate() { endwin(); }

    inline void doupdate() { ::doupdate(); }

    inline int keypad(WINDOW *win, bool enable) { return ::keypad(win, enable); }

    inline int nodelay(WINDOW *win, bool enable) { return ::nodelay(win, enable); }

    inline int delwin(WINDOW *win) { return ::delwin(win); }

    inline int wresize(WINDOW *win, int lines, int columns) { return ::wresize(win, lines, columns); }

    inline int mvwin(WINDOW *win, int y, int x) { return ::mvwin(win, y, x); }

    inline int getkey(WINDOW *win) { return ::wgetch(win); }

    inline int wnoutrefresh(WINDOW *win) { return ::wnoutrefresh(win); }

    inline int werase(WINDOW *win) { return ::werase(win); }

    inline int get_height(WINDOW *win = stdscr)
    {
        int x, y;
        std::ignore = x;
        getmaxyx(win, y, x);
        return y;
    }

    inline int get_width(WINDOW *win = stdscr)
    {
        int x, y;
        std::ignore = y;
        getmaxyx(win, y, x);
        return x;
    }

    inline int mvprintn(WINDOW *win,
                        int x,
                        int y,
                        const std::string &str,
                        int n,
                        attribute attrs = attribute::none,
                        colour clr = colour::none)
    {
        wattron(win, clr | attrs);
        int retval = mvwaddnstr(win, y, x, str.c_str(), n);
        wattroff(win, clr | attrs);
        return retval;
    }

    inline int mvprint(int x, int y, const std::string &str, attribute attrs = attribute::none, colour clr = colour::none)
    {
        attron(clr | attrs);
        int retval = mvaddstr(y, x, str.c_str());
        attroff(clr | attrs);
        return retval;
    }

    inline int
    mvprint(WINDOW *win, int x, int y, const std::string &str, attribute attrs = attribute::none, colour clr = colour::none)
    {
        wattron(win, clr | attrs);
        int retval = mvwaddstr(win, y, x, str.c_str());
        wattroff(win, clr | attrs);
        return retval;
    }

} // namespace bookwyrm::tui::curses
