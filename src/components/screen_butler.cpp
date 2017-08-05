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

#include <termbox.h>

#include "components/screen_butler.hpp"

namespace butler {

screen_butler::screen_butler(vector<bookwyrm::item> &items)
    : items_(items), viewing_details_(false)
{
    /* Create the default menu screen and focus on it. */
    index_ = std::make_shared<screen::multiselect_menu>(items_);
    focused_ = index_;
    screens_.insert(index_);
}

void screen_butler::update_screens()
{
    tb_clear();

    if (!bookwyrm_fits()) {
        mvprintw(0, 0, "The terminal is too small. I can't fit!");
    } else {
        for (auto &screen : screens_)
           screen->update();

        print_footer();
    }

    tb_present();
}

void screen_butler::print_footer()
{
    mvprintw(0, tb_height() - 2, focused_->footer_info());
    mvprintwl(0, tb_height() - 1, "[ESC]Quit " + focused_->footer_controls(), TB_REVERSE | TB_BOLD);
}

void screen_butler::resize_screens()
{
    for (auto &screen : screens_)
        screen->on_resize();

    update_screens();
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
            resize_screens();
        } else if (ev.type == TB_EVENT_KEY) {
            if (ev.key == TB_KEY_ESC)
                return;

            /* When the terminal is too small, only allow quitting. */
            if (!bookwyrm_fits())
                continue;

            if (meta_action(ev.key, ev.ch) || focused_->action(ev.key, ev.ch))
                update_screens();
        }
    }
}

bool screen_butler::meta_action(const uint16_t &key, const uint32_t &ch)
{
    switch (ch) {
        case 'l':
            return open_details();
        case 'h':
            return close_details();
    }

    switch (key) {
        case TB_KEY_CTRL_L:
            /* Update the screens, done in calling function. */
            return true;
        case TB_KEY_ARROW_RIGHT:
            return open_details();
        case TB_KEY_ARROW_LEFT:
            return close_details();
    }

    return false;
}

bool screen_butler::open_details()
{
    if (viewing_details_) return false;

    /* How much space will the detail menu take up? */
    int height;
    std::tie(index_scrollback_, height) = index_->compress();

    details_ = std::make_shared<screen::item_details>(index_->selected_item(), height);
    focused_ = details_;
    screens_.insert(details_);

    viewing_details_ = true;
    return true;
}

bool screen_butler::close_details()
{
    if (!viewing_details_) return false;

    focused_ = index_;
    screens_.erase(details_);

    /* Give back the space the detail menu too up to the index menu. */
    index_->decompress(index_scrollback_);
    index_scrollback_ = -1;

    viewing_details_ = false;
    return true;
}

void screen_butler::mvprintw(int x, const int y, const string_view &str, const uint16_t attrs)
{
    for (const uint32_t &ch : str)
        tb_change_cell(x++, y, ch, attrs, 0);
}

void screen_butler::mvprintwl(int x, const int y, const string_view &str, const uint16_t attrs)
{
    for (int i = 0; i < x; i++)
        tb_change_cell(i, y, ' ', attrs, 0);

    mvprintw(x, y, str, attrs);

    for (int i = x + str.length(); i < tb_width() - 1; i++)
        tb_change_cell(i, y, ' ', attrs, 0);
}

/* ns butler */
}

namespace tui {

std::shared_ptr<butler::screen_butler> make_with(butler::script_butler &butler, vector<py::module> &sources)
{
    auto tui = std::make_shared<butler::screen_butler>(butler.results());
    butler.set_screens(tui);
    butler.async_search(sources); // Watch out, it's hot!
    return tui;
}

/* ns tui */
}

