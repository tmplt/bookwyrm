#include "screens/item_details.hpp"
#include "../string.hpp"

namespace bookwyrm::tui::screen {

    item_details::item_details(const core::item &item, int padding_top)
        : base(padding_top, default_padding_bot, 0, 0), item_(item)
    {
    }

    void item_details::paint()
    {
        erase();
        print_borders();
        print_details();
        refresh();
    }

    std::string item_details::controls_legacy() const { return "[h/<-]Close details"; }

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

        /* Find the longest string, and create an indent length by adding 4. */
        // clang-format off
        const auto indent = std::max_element(cbegin(v), cend(v), [](const auto &a, const auto &b) {
            return a.first.length() < b.first.length();
        })->first.length() + 4;
        // clang-format on

        /* Align all fields to calculated indent. */
        int y = 1;
        for (const auto &p : v) {
            print(0, y, p.first + ':', attribute::bold);
            print(indent, y, p.second.get());

            y += std::ceil(static_cast<double>(indent + p.second.get().length()) / get_width());
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
