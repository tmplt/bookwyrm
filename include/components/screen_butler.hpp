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
#include "components/script_butler.hpp"
#include "screens/base.hpp"
#include "screens/multiselect_menu.hpp"

namespace bookwyrm {

class screen_butler;
class script_butler;

namespace tui {

std::shared_ptr<screen_butler> make_with(script_butler &butler, vector<py::module> &sources);

/* ns tui */
}

class screen_butler {
public:
    /*
     * We'll want to know the items when we create new screens.
     * WARN: this constructor should only be used in make_with() above.
     */
    explicit screen_butler(vector<item> &items);

    /* Repaint all the screens that need updating. */
    void update_screens();

    /*
     * Display the TUI and let the user enter input.
     * The input is forwarded to the appropriate screen.
     */
    void display();

private:
    vector<item> const &items_;
    vector<std::shared_ptr<screen_base>> screens_;
    std::shared_ptr<screen_base> focused_;
};

/* ns bookwyrm */
}
