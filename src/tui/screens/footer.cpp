#include <fmt/format.h>

#include "../string.hpp"
#include "screens/footer.hpp"

namespace bookwyrm::tui::screen {

    footer::footer() : base(-3, 0, 0, 0) {}

    void footer::paint()
    {
        /* Print the number of running plugins and found items. */
        if (plugin_count_ == 0) {
            print(0, 1, fmt::format("Search finished: I found {} items.", item_count_));
        } else {
            print(0,
                  1,
                  fmt::format(
                      "Searching with {} running plugins... I have found {} items thus far.", plugin_count_, item_count_));
        }

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
    }

} // namespace bookwyrm::tui::screen
