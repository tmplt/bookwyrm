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

#include "tui.hpp"
#include "utils.hpp"

namespace bookwyrm {

tui::tui(vector<core::item> &items, logger_t logger)
    : items_(items), logger_(logger), viewing_details_(false)
{
    /* Create the log screen. */
    log_ = std::make_shared<screen::log>();

    /* And create the default menu screen and focus on it. */
    index_ = std::make_shared<screen::multiselect_menu>(items_);
    focused_ = index_;
}

void tui::log(const core::log_level level, const string message)
{
    using spdlvl = spdlog::level::level_enum;
    using belvl  = core::log_level;

    /* Can we perhaps construct a spdlvl with the underlying int from level? */
    spdlvl frontend_lvl;

    switch (level) {
        case belvl::trace:
            frontend_lvl = spdlvl::trace;
            break;
        case belvl::debug:
            frontend_lvl = spdlvl::debug;
            break;
        case belvl::info:
            frontend_lvl = spdlvl::info;
            break;
        case belvl::warn:
            frontend_lvl = spdlvl::warn;
            break;
        case belvl::err:
            frontend_lvl = spdlvl::err;
            break;
        case belvl::critical:
            frontend_lvl = spdlvl::critical;
            break;
        case belvl::off:
            frontend_lvl = spdlvl::off;
    }

    logger_->log(frontend_lvl, message);
}

void tui::repaint_screens()
{
    tb_clear();

    if (!bookwyrm_fits()) {
        wprint(0, 0, "The terminal is too small. I don't fit!");
    } else if (is_log_focused()) {
        log_->paint();
        print_footer();
    } else {
        index_->paint();

        if (viewing_details_)
            details_->paint();

        print_footer();
    }

    tb_present();
}

void tui::print_footer()
{
    const auto print_right_align = [this](int y, string &&str, const colour attrs = colour::none) {
        this->wprint(tb_width() - str.length(), y, str, attrs);
    };

    /* Screen info bar. */
    wprint(0, tb_height() - 2, focused_->footer_info());

    /* Scroll percentage, if any. */
    if (int perc = focused_->scrollpercent(); perc > -1)
        print_right_align(tb_height() - 2, fmt::format("({}%)", perc));

    /* Screen controls info bar. */
    wprintcont(0, tb_height() - 1, "[ESC]Quit [TAB]Toggle log " + focused_->controls_legacy(),
            attribute::reverse | attribute::bold);

    /* Any unseen logs? */
    if (logger_->has_unread_logs()) {
        print_right_align(tb_height() - 1, " You have unread logs! ",
                utils::to_colour(logger_->worst_unread()) | attribute::reverse | attribute::bold);
    }
}

void tui::resize_screens()
{
    index_->on_resize();
    log_->on_resize();

    /* Resizing item_details not yet supported. */

    repaint_screens();
}

bool tui::display()
{
    repaint_screens();

    struct keys::event ev;
    while (keys::poll_event(ev)) {
        if (ev.type == type::resize) {
            close_details();
            resize_screens();
        } else if (ev.type == type::key_press) {
            if (ev.key == key::escape)
                return false;

            /* When the terminal is too small, only allow quitting and window resizing. */
            if (!bookwyrm_fits())
                continue;

            if (ev.key == key::enter)
                return true;

            if (meta_action(ev.key, ev.ch) || focused_->action(ev.key, ev.ch))
                repaint_screens();
        }
    }

    throw program_error("unable to poll input");
}

vector<core::item> tui::get_wanted_items()
{
    vector<core::item> items;

    for (int idx : index_->marked_items())
        items.push_back(items_[idx]);

    return items;
}

bool tui::bookwyrm_fits()
{
    /*
     * I planned to use the classical 80x24, but multiselect_menu is
     * in its current form useable in terminals much smaller
     * than that.
     */
    return tb_width() >= 50 && tb_height() >= 10;
}

bool tui::meta_action(const key &key, const uint32_t &ch)
{
    switch (ch) {
        case 'l':
            return open_details();
        case 'h':
            return close_details();
    }

    switch (key) {
        case key::ctrl_l:
            /* Repaint the screens, done in calling function. */
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

bool tui::open_details()
{
    if (viewing_details_ || index_->item_count() == 0) return false;

    /* How much space will the detail menu take up? */
    int height;
    std::tie(index_scrollback_, height) = index_->compress();

    details_ = std::make_shared<screen::item_details>(index_->selected_item(), tb_height() - height - 1);
    focused_ = details_;

    viewing_details_ = true;
    return true;
}

bool tui::close_details()
{
    if (!viewing_details_) return false;

    focused_ = index_;

    /* Give back the space the detail menu too up to the index menu. */
    index_->decompress(index_scrollback_);
    index_scrollback_ = -1;

    viewing_details_ = false;
    return true;
}

bool tui::toggle_log()
{
    if (focused_ != log_) {
        last_ = focused_;
        focused_ = log_;

        logger_->flush_to_screen();
    } else {
        focused_ = last_;
    }

    return true;
}

void tui::wprint(int x, const int y, const string_view &str, const colour attrs)
{
    for (const uint32_t &ch : str)
        tb_change_cell(x++, y, ch, static_cast<colour_t>(attrs), 0);
}

void tui::wprintcont(int x, const int y, const string_view &str, const colour attrs)
{
    for (int i = 0; i < x; i++)
        tb_change_cell(i, y, ' ', static_cast<colour_t>(attrs), 0);

    wprint(x, y, str, attrs);

    for (int i = x + str.length(); i < tb_width(); i++)
        tb_change_cell(i, y, ' ', static_cast<colour_t>(attrs), 0);
}

std::shared_ptr<tui> make_tui_with(core::plugin_handler &plugin_handler, logger_t &logger)
{
    plugin_handler.load_plugins();
    auto t = std::make_shared<tui>(plugin_handler.results(), logger);
    plugin_handler.set_frontend(t);
    logger->set_tui(t);
    plugin_handler.async_search();
    return t;
}

/* ns bookwyrm */
}
