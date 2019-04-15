#include "screens/item_details.hpp"
#include "../string.hpp"

namespace bookwyrm::tui::screen {

    item_details::item_details(const core::item &item, int padding_top)
        : base(padding_top, default_padding_bot, 0, 0), item_(item)
    {
    }

    bool item_details::action(const int ch)
    {
        std::ignore = ch;

        /* No actions for this screen yet. */

        return false;
    }

    void item_details::paint()
    {
        erase();
        print_borders();
        print_details();
        refresh();
    }

    std::string item_details::footer_info() const
    {
        /* TODO: remove this */
        return fmt::format("DEBUG: padding top: {}, height: {}", pads_.top, get_height());
    }

    int item_details::scrollpercent() const
    {
        /* stub */
        return 42;
    }

    void item_details::print_borders()
    {
        const auto print_line = [this](int y) {
            for (int x = 0; x < get_width(); x++)
                print(x, y, rune::em_dash);
        };

        print_line(0);
        print_line(get_height() - 1);
    }

    void item_details::print_details()
    {
        auto to_str = [](int i) -> const std::string { return (i == core::empty) ? "" : std::to_string(i); };

        // clang-format off
        const std::string authors = vector_to_string(item_.nonexacts.authors),
              uris = vector_to_string(item_.misc.mirrors),
              isbns = vector_to_string(item_.misc.isbns),
              year = to_str(item_.exacts.year),
              size = to_str(item_.exacts.size) + " B", // size is recorded in bytes
              pages = to_str(item_.exacts.pages),
              volume = to_str(item_.exacts.volume),
              number = to_str(item_.exacts.number);
        // clang-format on

        using pair = std::pair<std::string, std::reference_wrapper<const std::string>>;
        const std::vector<pair> v = {
            {"Authors", authors},
            {"Title", item_.nonexacts.title},
            {"Serie", item_.nonexacts.series},
            {"Publisher", item_.nonexacts.publisher},
            {"Edition", item_.nonexacts.edition},
            {"Year", year},

            {"Journal", item_.nonexacts.journal},
            {"Extension", item_.exacts.extension},

            {"Size", size}, // TODO: print as read if very large
            {"Pages", pages},
            {"Volume", volume},
            {"Number", number},
            {"Mirrors", uris},
            {"Source", item_.misc.origin_plugin},
            {"ISBNs", isbns},
        };

        /* Find the longest string... */
        size_t len = 0;
        for (const auto &p : v)
            len = std::max(p.first.length(), len);

        /*
         * ... which we use to distance field title and field value.
         * (A magic 4 added to x to emulate a tab).
         */
        int y = 1;
        for (const auto &p : v) {
            print(0, y, p.first + ':', attribute::bold);
            print(static_cast<int>(len + 4), y++, p.second.get());

            /* How many lines did the string take up? */
            if (int lines = (static_cast<int>(len + 4) + p.second.get().length()) / get_width(); lines > 1) {
                y += lines;
            }
        }
    }

    void item_details::print_desc(int &y, std::string str)
    {
        int x = 0;
        const auto words = split_string(str);

        auto word_fits = [this, &x](const std::string &str) -> bool {
            return static_cast<size_t>(get_width() - x) > str.length();
        };

        for (auto word = words.cbegin(); word != words.cend(); ++word) {
            if (!word_fits(*word)) {
                if (y == get_height() - 1) {
                    /* No more lines to draw on; can't fit any more. */

                    if (word != words.cend() - 1) {
                        /* We haven't printed the whole description yet. */

                        /*
                         * Make sure the dots are printed in the screen.
                         * Subtracts an additional 1 to overwrite the space
                         * from the last word.
                         */
                        print(word_fits("...") ? --x : x - 4, y, "...");
                    }

                    return;
                }

                ++y;
                x = 0;
            }

            print(x, y, *word + ' ');
            x += static_cast<int>(word->length()) + 1;
        }
    }

} // namespace bookwyrm::tui::screen
