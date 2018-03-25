#pragma once

#include <ncurses.h>

namespace curses {

/* inline int get_height() */
/* { */
/*     int x, y; */
/*     (void)x; */
/*     getmaxyx(stdscr, y, x); */
/*     return y; */
/* } */

/* inline int get_width() */
/* { */
/*     int x, y; */
/*     (void)y; */
/*     getmaxyx(stdscr, y, x); */
/*     return x; */
/* } */

/* inline void refresh() */
/* { */
/*     ::refresh(); */
/* } */

/* inline void refresh(WINDOW *win) */
/* { */
/*     ::wrefresh(win); */
/* } */

inline int getch()
{
    return ::getch();
}

/* inline void erase() */
/* { */
/*     ::erase(); */
/* } */

/* inline void erase(WINDOW *win) */
/* { */
/*     ::werase(win); */
/* } */

/* ns curses */
}
