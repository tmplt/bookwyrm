#pragma once

#include "screens/base.hpp"

namespace bookwyrm::tui::screen {

    class footer : public base {
    public:
        explicit footer();

        void paint() override;

        void prepare(int plugin_count,
                     int item_count,
                     int scroll_perc,
                     std::string screen_controls,
                     std::optional<core::log_level> worst_unread)
        {
            plugin_count_ = plugin_count;
            item_count_ = item_count;
            scroll_perc_ = scroll_perc;
            screen_controls_ = screen_controls;
            worst_unread_ = worst_unread;
        }

    private:
        int plugin_count_ = 0;
        int item_count_ = 0;
        int scroll_perc_ = -1;
        std::string screen_controls_;
        std::optional<core::log_level> worst_unread_;
    };

} // namespace bookwyrm::tui::screen
