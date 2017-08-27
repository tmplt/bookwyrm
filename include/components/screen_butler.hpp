/*
 * Copyright (C) 2017 Tmplt <tmplt@dragons.rocks>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "common.hpp"
#include "item.hpp"
#include "python.hpp"
#include "colours.hpp"
#include "components/script_butler.hpp"
#include "components/logger.hpp"
#include "screens/base.hpp"
#include "screens/multiselect_menu.hpp"
#include "screens/item_details.hpp"
#include "screens/log.hpp"

namespace butler {

class script_butler;

/*
 * Another butler. This one handles whatever screens we want to show the user,
 * as well as which of them to update. User input post-cli is also handled here,
 * which is forwarded to the currently focused screen unless it was used to
 * manage screens.
 */
class screen_butler {
public:
    /* WARN: this constructor should only be used in make_with() above. */
    explicit screen_butler(vector<bookwyrm::item> &items, bool &tui_up, logger_t logger);
    ~screen_butler();

    /* Update (redraw) all screens that need updating. */
    void update_screens();
    void print_footer();

    /*
     * Display the TUI and let the user enter input.
     * The input is forwarded to the appropriate screen.
     */
    void display();

private:
    /* We'll want to know the items when we create new screens. */
    vector<bookwyrm::item> const &items_;

    bool &tui_up_;

    std::shared_ptr<screen::multiselect_menu> index_;
    std::shared_ptr<screen::log> log_;

    std::shared_ptr<screen::item_details> details_;
    std::shared_ptr<screen::base> focused_;

    /* Is a screen::item_details open? */
    bool viewing_details_;

    /* When we close the screen::item_details, how much does the index menu scroll back? */
    int index_scrollback_ = -1;

    /* Returns true if the bookwyrm fits in the current terminal window. */
    static bool bookwyrm_fits();

    /* Manage screens. Return true if an action was performed. */
    bool meta_action(const key &key, const uint32_t &ch);

    /*
     * Open a screen::item_details for the currently selected item in the index menu.
     * Returns true if the operation was successful (no detail screen is already open).
     */
    bool open_details();

    /* And close it. Return true if the operation was successful. */
    bool close_details();

    bool toggle_log();

    void resize_screens();

    /* Non-asserting copy from screen::base. */
    static void mvprintw(int x, const int y, const string_view &str, const colour attrs = colour::white);

    /*
     * Print passed string starting from (x, y) along the x-axis.
     * All other cells on the same line will be empty (' ') with
     * attrs applied.
     */
    static void mvprintwl(int x, const int y, const string_view &str, const colour attrs = colour::white);
    static void mvprintwl(int x, const int y, const string_view &str, const attribute attr)
    {
        mvprintwl(x, y, str, colour::white | attr);
    }
};

/* ns butler */
}

namespace tui {

/*
 * Yes, a factory. But we need it to "link" the two butlers together.
 * (The script_butler tells the screen_butler when to update all screens.)
 */
std::shared_ptr<butler::screen_butler> make_with(butler::script_butler &butler, vector<py::module> &sources, bool &tui_up, logger_t &logger);

/* ns tui */
}
