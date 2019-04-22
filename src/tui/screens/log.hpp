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
        std::optional<entry_ri> detached_at_;

        void maybe_update_detached(std::function<void()> &&fun);
        void print_entry(int &y, const entry_ri entry);
    };

} // namespace bookwyrm::tui::screen
