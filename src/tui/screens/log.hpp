#pragma once

#include <optional>

#include "plugin_handler.hpp"
#include "screens/base.hpp"

namespace bookwyrm::tui::screen {

    class log : public base {
    public:
        explicit log(const core::log_level wanted_level, std::function<bool(void)> &&predicate);

        void paint() override;
        void toggle_action() override;
        void move(move_direction dir) override;

        std::string footer_info() const override;
        int scrollpercent() const override;
        std::string controls_legacy() const override;

        void log_entry(core::log_level level, std::string msg);

        void mark_read();

        std::optional<core::log_level> worst_unread() const;
        std::vector<core::log_pair> unread_logs() const;

    private:
        const core::log_level wanted_level_;
        std::function<bool(void)> is_log_focused_;

        std::vector<core::log_pair> entries_, unread_entries_;
        using entry_ri = decltype(entries_.crbegin());
        using entry_i = decltype(entries_.cbegin());
        std::optional<entry_ri> detached_at_;

        /*
         * Entries will be modified both by calls to `log_entry()` from plugins
         * and by calls to `mark_read()` when the log is toggled.
         * Protects `{,unread_}entries_` and `detached_at_` above.
         */
        mutable std::mutex entries_mutex_;

        void maybe_update_detached(std::function<void()> &&fun);
        int capacity(const entry_ri &start) const;
        int fcapacity(const entry_i &start) const;
        void print_entry(int &y, const entry_ri entry);
    };

} // namespace bookwyrm::tui::screen
