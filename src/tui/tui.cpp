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
        std::lock_guard<std::mutex> guard(paint_mutex_);

        footer_->prepare(backend_->running_plugins(),
                         index_->item_count(),
                         focused_->scrollpercent(),
                         focused_->controls_legacy(),
                         log_->worst_unread());

        if (!bookwyrm_fits()) {
            curses::werase(stdscr);
            curses::mvprint(0, 0, "The terminal is too small. I don't fit!");
        } else if (is_log_focused()) {
            log_->paint();
            footer_->paint();
        } else {
            index_->paint();

            if (viewing_details_) {
                details_->paint();
            }

            footer_->paint();
        }

        curses::doupdate();
    }

    bool tui::is_log_focused() const { return focused_ == log_; }

    void tui::log(const core::log_level level, const std::string message)
    {
        /* Forward to log screen */
        log_->log_entry(level, message);
        update();
    }

    void tui::resize_screens()
    {
        close_details();

        {
            std::lock_guard<std::mutex> guard(paint_mutex_);
            index_->on_resize();
            log_->on_resize();
            footer_->on_resize();
        }

        update();
    }

    bool tui::display()
    {
        /* Create the default menu screen and focus on it. */
        index_ = std::make_shared<screen::index>(backend_->search_results());
        footer_ = std::make_unique<screen::footer>();
        focused_ = index_;

        update();

        while (true) {
            const key ch = std::invoke([this]() {
                std::lock_guard<std::mutex> guard(paint_mutex_);

                /*
                 * Because we use ncurses WINDOWs and have parallel threads that all update the TUI,
                 * we will eventually end up in a situation where a screen has been resized by an external
                 * event (SIGWINCH) but have not been handled by ncurses internals. In such a case, all
                 * affected WINDOWs are updated by getch(3), which is a race condition with update()
                 * called from other threads. Hence, we must have exclusive TUI access before the call.
                 * Additionally, the call must be non-blocking.
                 *
                 * Can this be improved by using ncurses PADs instead?
                 */
                return static_cast<key>(curses::getkey());
            });

            if (ch == key::resize) {
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

            if (meta_action(ch) || focused_->action(ch)) {
                update();
            }
        }
    }

    std::optional<std::vector<core::item>> tui::get_wanted_items()
    {
        /* Display the TUI, allowing the user to select any items. */
        if (display() == false)
            return std::nullopt;

        std::vector<core::item> items;

        for (int idx : index_->marked_items())
            items.push_back(*std::next(backend_->search_results().cbegin(), idx));

        return items;
    }

    std::vector<core::log_pair> tui::unread_logs() const { return std::move(log_->unread_logs()); }

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

} // namespace bookwyrm::tui
