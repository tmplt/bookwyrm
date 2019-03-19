#include <iostream>

#include "curses_wrap.hpp"
#include "tui.hpp"

#define BOOKWYRM_MINIMUM_HEIGHT 10
#define BOOKWYRM_MINIMUM_WIDTH 50

namespace bookwyrm::tui {

    tui::tui(std::shared_ptr<core::backend> backend, bool log_debug) : viewing_details_(false), backend_(backend)
    {
        /* Create the log screen. */
        log_ = std::make_shared<screen::log>(log_debug ? core::log_level::debug : core::log_level::warn,
                                             [this]() { return is_log_focused(); });
        log_->log_entry(core::log_level::debug, "the mighty bookwyrm hath been summoned!");
    }

    void tui::update()
    {
        /* Don't paint anything unless the index menu exists (when attaching tui to backend) */
        if (!index_) {
            return;
        }

        /* Repaint all active screens. */
        std::lock_guard<std::mutex> guard(tui_mutex_);
        curses::erase();

        if (!bookwyrm_fits()) {
            print(0, 0, "The terminal is too small. I don't fit!");
        } else if (is_log_focused()) {
            log_->paint();
            print_footer();
        } else {
            index_->paint();

            if (viewing_details_)
                details_->paint();

            print_footer();
        }

        curses::refresh();
    }

    bool tui::is_log_focused() const { return focused_ == log_; }

    void tui::log(const core::log_level level, const std::string message)
    {
        /* Forward to log screen */
        log_->log_entry(level, message);
        update();
    }

    void tui::print_footer()
    {
        const auto print_right_align = [this](int y, std::string &&str, const colour attrs = colour::none) {
            this->print(curses::get_width() - str.length(), y, str, attrs);
        };

        /* Print number of running plugins and screen info bar. */
        if (int plugins = backend_->running_plugins(); plugins == 0) {
            print(0, curses::get_height() - 2, fmt::format("Search finished, I have found {} items.", index_->item_count()));
        } else {
            print(0,
                  curses::get_height() - 2,
                  fmt::format("Searching with {} plugins... "
                              "I have found {} items thus far.",
                              plugins,
                              index_->item_count()));
        }

        /* Scroll percentage, if any. */
        if (int perc = focused_->scrollpercent(); perc > -1)
            print_right_align(curses::get_height() - 2, fmt::format("({}%)", perc));

        /* Screen controls info bar. */
        printcont(0,
                  curses::get_height() - 1,
                  "[q]Quit [TAB]Toggle log " + focused_->controls_legacy(),
                  attribute::reverse | attribute::bold);

        /* Any unseen logs? */
        if (const auto worst_unread = log_->worst_unread(); worst_unread.has_value()) {
            print_right_align(curses::get_height() - 1,
                              " You have unread logs! ",
                              to_colour(*worst_unread) | attribute::reverse | attribute::bold);
        }
    }

    void tui::resize_screens()
    {
        index_->on_resize();
        log_->on_resize();

        /* Resizing item_details not yet supported. */

        update();
    }

    bool tui::display()
    {
        /* Create the default menu screen and focus on it. */
        index_ = std::make_shared<screen::multiselect_menu>(backend_->search_results());
        focused_ = index_;

        update();

        while (true) {
            const key ch = static_cast<key>(curses::getkey());

            if (ch == key::resize) {
                close_details();
                resize_screens();
                continue;
            }

            if (ch == 'q')
                return false;

            /* When the terminal is too small, only allow quitting and window resizing.
             */
            if (!bookwyrm_fits())
                continue;

            if (ch == key::enter)
                return true;

            if (meta_action(ch) || focused_->action(ch))
                update();
        }
    }

    std::optional<std::vector<core::item>> tui::get_wanted_items()
    {
        /* Display the TUI, aloowing the user to select any items. */
        if (display() == false)
            return std::nullopt;

        std::vector<core::item> items;

        for (int idx : index_->marked_items())
            items.push_back(*std::next(backend_->search_results().cbegin(), idx));

        return items;
    }

    bool tui::bookwyrm_fits()
    {
        return (curses::get_width() >= BOOKWYRM_MINIMUM_WIDTH && curses::get_height() >= BOOKWYRM_MINIMUM_HEIGHT);
    }

    bool tui::meta_action(const int ch)
    {
        switch (ch) {
        case 'l':
        case key::arrow_right:
            if (is_log_focused())
                return false;
            return open_details();
        case 'h':
        case key::arrow_left:
            if (is_log_focused())
                return false;
            return close_details();
        case key::tab:
            return toggle_log();
        default:
            return false;
        }
    }

    bool tui::open_details()
    {
        if (viewing_details_ || index_->item_count() == 0)
            return false;

        /* How much space will the detail menu take up? */
        int height;
        std::tie(index_scrollback_, height) = index_->compress();

        details_ = std::make_shared<screen::item_details>(index_->selected_item(), curses::get_height() - height - 1);
        focused_ = details_;

        viewing_details_ = true;
        return true;
    }

    bool tui::close_details()
    {
        if (!viewing_details_)
            return false;

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
        } else {
            focused_ = last_;
        }

        return true;
    }

    void tui::print(int x, const int y, const std::string &str, const colour attrs)
    {
        curses::mvprint(x, y, str, attribute::none, attrs);
    }

    void tui::printcont(int x, const int y, const std::string &str, const colour attrs)
    {
        curses::mvprint(x, y, str, attribute::none, attrs);

        for (int i = x + str.length(); i < curses::get_width(); i++)
            curses::mvprint(i, y, " ", attribute::none, attrs);
    }

} // namespace bookwyrm::tui
