#include <fmt/format.h>

#include "../string.hpp"
#include "screens/footer.hpp"

namespace bookwyrm::tui::screen {

    footer::footer() : base(-3, 0, 0, 0) {}

    void footer::paint()
    {
        erase();

        /* Print the contextual info string. */
        print(0, 1, footer_info_);

        /* Print scroll percentage, if any. */
        if (scroll_perc_ > -1) {
            print_right_align(1, fmt::format("({})", scroll_perc_));
        }

        /* Print the screen controls. */
        printcont(0,
                  2,
                  fmt::format("[q]Quit [TAB]Toggle log {} ", screen_controls_),
                  colour::none | attribute::reverse | attribute::bold);

        /* Print a notification for unseen logs, if any. */
        if (worst_unread_.has_value()) {
            print_right_align(
                2, " You have unread logs! ", to_colour(worst_unread_.value()) | attribute::reverse | attribute::bold);
        }

        refresh();
    }

    void footer::prepare(std::string footer_info,
                         int scroll_perc,
                         std::string screen_controls,
                         std::optional<core::log_level> worst_unread)
    {
        footer_info_ = footer_info;
        scroll_perc_ = scroll_perc;
        screen_controls_ = screen_controls;
        worst_unread_ = worst_unread;
    }

} // namespace bookwyrm::tui::screen
