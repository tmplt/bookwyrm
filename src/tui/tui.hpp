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

    /* Translates a level enum to a matching colour. */
    colour to_colour(core::log_level lvl);

    struct logger {
        explicit logger(core::log_level wanted_level, std::function<bool(void)> &&predicate)
            : wanted_level_(wanted_level), is_log_focused(predicate)
        {
        }

        /* Flush buffer to stderr */
        ~logger();

        void log(const core::log_level level, std::string message);

        inline void trace(const std::string msg) { log(core::log_level::trace, "trace: " + msg); }
        inline void debug(const std::string msg) { log(core::log_level::debug, "debug: " + msg); }
        inline void info(const std::string msg) { log(core::log_level::info, "info: " + msg); }
        inline void warn(const std::string msg) { log(core::log_level::warn, "warning: " + msg); }
        inline void err(const std::string msg) { log(core::log_level::err, "error: " + msg); }
        inline void critical(const std::string msg) { log(core::log_level::critical, "critical: " + msg); }

        inline bool has_unread_logs() const { return !buffer_.empty(); }

        core::log_level worst_unread() const;

        void flush_to_screen();
        void set_screen(std::shared_ptr<screen::log> screen) { screen_ = screen; }

    private:
        const core::log_level wanted_level_;
        using buffer_pair = std::pair<const core::log_level, const std::string>;
        std::vector<buffer_pair> buffer_;
        std::mutex log_mutex_;

        std::weak_ptr<screen::log> screen_;
        std::function<bool(void)> is_log_focused;
    };

    class tui : public core::frontend {
    public:
        explicit tui(std::set<core::item> &items, bool debug_log, const std::atomic<size_t> &running_plugins);

        inline void update();

        /* Send a log entry to the log screen. */
        void log(const core::log_level level, const std::string message);

        /* Repaint all screens that need updating. */
        void repaint_screens();

        /*
         * Display the TUI and let the user enter input.
         * The input is forwarded to the appropriate screen.
         * Returns false if user wants the program to exit without downloading
         * anything. Returns true otherwise.
         */
        bool display();

        /* Take ownership of the wanted items and move them to the caller. */
        std::vector<core::item> get_wanted_items();

        /* Draw the context sensitive footer. */
        void print_footer();

        bool is_log_focused() const;

    private:
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

        /* Forwarded to the multiselect menu. */
        std::set<core::item> const &items_;
        std::mutex tui_mutex_;

        const std::atomic<size_t> &running_plugins_;

        std::unique_ptr<logger> logger_;

        std::shared_ptr<screen::multiselect_menu> index_;
        std::shared_ptr<screen::item_details> details_;
        std::shared_ptr<screen::log> log_;

        std::shared_ptr<screen::base> focused_, last_;
    };

} // namespace bookwyrm::tui
