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

screen_butler::screen_butler(vector<bookwyrm::item> &items, bool &tui_up)
    : items_(items), tui_up_(tui_up), viewing_details_(false)
{
    /*
     * Let the logger know it'll have to wait until program termination
     * to print its messages.
     */
    tui_up_ = true;

    /* Create the log screen. */
    log_ = std::make_shared<screen::log>();

    /* And create the default menu screen and focus on it. */
    index_ = std::make_shared<screen::multiselect_menu>(items_);
    focused_ = index_;
}

screen_butler::~screen_butler()
{
    tui_up_ = false;
}

void screen_butler::update_screens()
{
    tb_clear();

    if (!bookwyrm_fits()) {
        mvprintw(0, 0, "The terminal is too small. I don't fit!");
    } else if (focused_ == log_) {
        log_->update();
        print_footer();
    } else {
        index_->update();

        if (viewing_details_)
            details_->update();

        print_footer();
    }

    tb_present();
}

void screen_butler::print_footer()
{
    const auto print_right_align = [this](int y, string &&s) {
        mvprintw(tb_width() - s.length(), y, s);
    };

    mvprintw(0, tb_height() - 2, focused_->footer_info());
    if (int perc = focused_->scrollperc(); perc > -1)
        print_right_align(tb_height() - 2, fmt::format("({}%)", perc));

    mvprintwl(0, tb_height() - 1, "[ESC]Quit [TAB]Toggle log " + focused_->footer_controls(),
            attribute::reverse | attribute::bold);
}

void screen_butler::resize_screens()
{
    index_->on_resize();
    log_->on_resize();

    /* Resizing item_details not yet supported. */

    update_screens();
}

void screen_butler::display()
{
    update_screens();

    /* Let the source threads free. */
    py::gil_scoped_release nogil;

    struct keys::event ev;
    while (keys::poll_event(ev)) {
        if (ev.type == type::resize) {
            close_details();
            resize_screens();
        } else if (ev.type == type::key_press) {
            if (ev.key == key::escape)
                return;

            /* When the terminal is too small, only allow quitting. */
            if (!bookwyrm_fits())
                continue;

            if (meta_action(ev.key, ev.ch) || focused_->action(ev.key, ev.ch))
                update_screens();
        }
    }
}

bool screen_butler::bookwyrm_fits()
{
    /*
     * I planned to use the classical 80x24, but multiselect_menu is
     * in its current form useable in terminals much smaller
     * than that.
     */
    return tb_width() >= 50 && tb_height() >= 10;
}

bool screen_butler::meta_action(const key &key, const uint32_t &ch)
{
    switch (ch) {
        case 'l':
            return open_details();
        case 'h':
            return close_details();
    }

    switch (key) {
        case key::ctrl_l:
            /* Update the screens, done in calling function. */
            return true;
        case key::arrow_right:
            return open_details();
        case key::arrow_left:
            return close_details();
        case key::tab:
            return toggle_log();
        default:
            return false;
    }
}

bool screen_butler::open_details()
{
    if (viewing_details_) return false;

    /* How much space will the detail menu take up? */
    int height;
    std::tie(index_scrollback_, height) = index_->compress();

    details_ = std::make_shared<screen::item_details>(index_->selected_item(), tb_height() - height - 1);
    focused_ = details_;

    viewing_details_ = true;
    return true;
}

bool screen_butler::close_details()
{
    if (!viewing_details_) return false;

    focused_ = index_;

    /* Give back the space the detail menu too up to the index menu. */
    index_->decompress(index_scrollback_);
    index_scrollback_ = -1;

    viewing_details_ = false;
    return true;
}

bool screen_butler::toggle_log()
{
    if (focused_ != log_) {
        last_ = focused_;
        focused_ = log_;
    } else {
        assert(last_ != nullptr);
        focused_ = last_;
    }

    return true;
}

void screen_butler::mvprintw(int x, const int y, const string_view &str, const colour attrs)
{
    for (const uint32_t &ch : str)
        tb_change_cell(x++, y, ch, static_cast<colour_t>(attrs), 0);
}

void screen_butler::mvprintwl(int x, const int y, const string_view &str, const colour attrs)
{
    for (int i = 0; i < x; i++)
        tb_change_cell(i, y, ' ', static_cast<colour_t>(attrs), 0);

    mvprintw(x, y, str, attrs);

    for (int i = x + str.length(); i < tb_width(); i++)
        tb_change_cell(i, y, ' ', static_cast<colour_t>(attrs), 0);
}

/* ns butler */
}

namespace tui {

std::shared_ptr<butler::screen_butler> make_with(butler::script_butler &butler, vector<py::module> &sources, bool &tui_up, logger_t &logger)
{
    auto tui = std::make_shared<butler::screen_butler>(butler.results(), tui_up);
    butler.set_screens(tui);
    logger->set_screen_butler(tui);
    butler.async_search(sources); // Watch out, it's hot!
    return tui;
}

/* ns tui */
}
