#pragma once


#include "core/plugin_handler.hpp"
#include "core/item.hpp"
#include "common.hpp"
#include "colours.hpp"
#include "logger.hpp"
#include "screens/base.hpp"
#include "screens/multiselect_menu.hpp"
#include "screens/item_details.hpp"
#include "screens/log.hpp"

/* Circular dependency guard. */
namespace logger { class bookwyrm_logger; }
using logger_t = std::shared_ptr<logger::bookwyrm_logger>;

namespace core { class plugin_handler; }

namespace bookwyrm {

class tui : public core::frontend {
public:
    void update()
    {
        repaint_screens();
    }

    void log(const core::log_level level, const string message);

    /* Send a log entry to the log screen. */
    void log(const spdlog::level::level_enum level, const string message)
    {
        log_->log_entry(level, message);
        repaint_screens();
    }

    /* WARN: this constructor should only be used in make_with() above. */
    explicit tui(vector<core::item> &items, logger_t logger);

    /* Repaint all screens that need updating. */
    void repaint_screens();

    /*
     * Display the TUI and let the user enter input.
     * The input is forwarded to the appropriate screen.
     * Returns false if user wants the program to exit without downloading anything.
     * Returns true otherwise.
     */
    bool display();

    /* Take ownership of the wanted items and move them to the caller. */
    vector<core::item> get_wanted_items();

    /* Draw the context sensitive footer. */
    void print_footer();

    bool is_log_focused() const
    {
        return focused_ == log_;
    }

private:
    /* Forwarded to the multiselect menu. */
    vector<core::item> const &items_;

    /* Used to flush stored logs to the log screen. */
    logger_t logger_;

    std::shared_ptr<screen::multiselect_menu> index_;
    std::shared_ptr<screen::item_details> details_;
    std::shared_ptr<screen::log> log_;

    std::shared_ptr<screen::base> focused_, last_;

    /* Is a screen::item_details open? */
    bool viewing_details_;

    /* When we close the screen::item_details, how much does the index menu scroll back? */
    int index_scrollback_ = -1;

    /* Returns false if bookwyrm doesn't fit in the terminal window. */
    static bool bookwyrm_fits();

    /* Manage screens. Return true if an action was performed. */
    bool meta_action(const int ch);

    /*
     * Open a screen::item_details for the currently selected item in the index menu.
     * Returns true if the operation was successful (no detail screen is already open).
     */
    bool open_details();

    /* And close it. Return true if the operation was successful. */
    bool close_details();

    bool toggle_log();

    void resize_screens();

    /* copy from screen::base. */
    static void wprint(int x, const int y, const string &str, const colour attrs = colour::white);

    /*
     * Print passed string starting from (x, y) along the x-axis.
     * All other cells on the same line will be empty (' ') with
     * attrs applied.
     */
    static void wprintcont(int x, const int y, const string &str, const colour attrs = colour::white);
    static void wprintcont(int x, const int y, const string &str, const attribute attr)
    {
        wprintcont(x, y, str, colour::white | attr);
    }
};

std::shared_ptr<tui> make_tui_with(core::plugin_handler &plugin_handler, logger_t &logger);

/* ns bookwyrm */
}


