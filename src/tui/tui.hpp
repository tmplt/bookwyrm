#pragma once

#include <functional>
#include <iostream>

#include "colours.hpp"
#include "item.hpp"
#include "plugin_handler.hpp"
#include "screens/base.hpp"
#include "screens/item_details.hpp"
#include "screens/log.hpp"
#include "screens/multiselect_menu.hpp"

namespace bookwyrm::tui {

    class tui : public core::frontend {
    public:
        explicit tui(std::shared_ptr<core::backend> backend, bool log_debug);

        /* Repaint all screens that need updating. */
        void update() override;

        /* Send a log entry to the log screen. */
        void log(const core::log_level level, const std::string message) override;

        std::optional<std::vector<core::item>> get_wanted_items();
        std::vector<core::log_pair> unread_logs() const;

    private:
        /*
         * Display the TUI and let the user enter input.
         * The input is forwarded to the appropriate screen.
         * Returns false if user wants the program to exit without downloading
         * anything. Returns true otherwise.
         */
        bool display();

        /* Draw the context sensitive footer. */
        void print_footer();

        bool is_log_focused() const;

        /* Returns false if bookwyrm doesn't fit in the terminal window. */
        static bool bookwyrm_fits();

        /* Manage screens. Return true if an action was performed. */
        bool meta_action(const int ch);

        /*
         * Open a screen::item_details for the currently selected item in the index
         * menu. Returns true if the operation was successful (no detail screen is
         * already open).
         */
        bool open_details();

        /* And close it. Return true if the operation was successful. */
        bool close_details();

        bool toggle_log();

        void resize_screens();

        static void print(int x, const int y, const std::string &str, const colour attrs = colour::white);

        /*
         * Print passed string starting from (x, y) along the x-axis.
         * All other cells on the same line will be empty (' ') with
         * attrs applied.
         */
        static void printcont(int x, const int y, const std::string &str, const colour attrs = colour::white);
        static inline void printcont(int x, const int y, const std::string &str, const attribute attr)
        {
            printcont(x, y, str, colour::white | attr);
        }

        /* Is a screen::item_details open? */
        bool viewing_details_;

        /* When we close the screen::item_details, how much does the index menu scroll
         * back? */
        int index_scrollback_ = -1;

        std::shared_ptr<core::backend> backend_;

        /* Forwarded to the multiselect menu. */
        std::mutex tui_mutex_;

        std::shared_ptr<screen::multiselect_menu> index_;
        std::shared_ptr<screen::item_details> details_;
        std::shared_ptr<screen::log> log_;

        std::shared_ptr<screen::base> focused_, last_;
    };

} // namespace bookwyrm::tui
