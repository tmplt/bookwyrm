#pragma once

#include "screens/base.hpp"

namespace bookwyrm::tui::screen {

    class footer : public base {
    public:
        explicit footer();

        void paint() override;

        void prepare(std::string footer_info,
                     int scroll_perc,
                     std::string screen_controls,
                     std::optional<core::log_level> worst_unread);

    private:
        std::string footer_info_;
        int scroll_perc_ = -1;
        std::string screen_controls_;
        std::optional<core::log_level> worst_unread_;
    };

} // namespace bookwyrm::tui::screen
