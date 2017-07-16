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

#include "components/screen_butler.hpp"

namespace bookwyrm {

namespace tui {

std::shared_ptr<screen_butler> make_with(script_butler &butler, vector<py::module> &sources)
{
    auto tui = std::make_shared<screen_butler>(butler.results());
    butler.set_screens(tui);
    butler.async_search(sources); // Watch out, it's hot!
    return tui;
}

/* ns tui */
}

screen_butler::screen_butler(vector<item> &items)
    : items_(items)
{
    /* Create the default screen and focus on it. */
    auto menu = std::make_shared<multiselect_menu>(items_);
    focused_ = menu;
    screens_.emplace_back(menu);
}

void screen_butler::update_screens()
{
    for (auto &screen : screens_)
        screen->update();
}

void screen_butler::display()
{
    /*
     * Let the source threads free.
     * This doesn't feel like the best place to have this,
     * but we do need to have a release in scope if we
     * ever want the threads to run.
     * TODO: find out if there is a better place for this.
     */
    py::gil_scoped_release nogil;

    struct tb_event ev;
    while (tb_poll_event(&ev)) {
        if (ev.type == TB_EVENT_RESIZE) {
            for (auto &screen : screens_)
                screen->on_resize();
        } else if (ev.type == TB_EVENT_KEY) {
            /* When the terminal is too small, only allow quitting. */
            if (!screen_base::bookwyrm_fits()) {
                if (ev.key == TB_KEY_ESC)
                    return;

                continue;
            }

            switch (ev.key) {
                case TB_KEY_ESC:
                    return;
                case TB_KEY_CTRL_L:
                    focused_->update();
            }

            focused_->action(ev.key, ev.ch);
        }
    }
}

/* ns bookwyrm */
}
