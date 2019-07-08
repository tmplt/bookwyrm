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
        print_details();
        print_borders();
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

        using pair = std::optional<std::pair<std::string, std::reference_wrapper<const std::string>>>;
        const std::vector<pair> v = {
            {{"Authors", authors}},
            {{"Title", item_.nonexacts.title}},
            {{"Series", item_.nonexacts.series}},
            {{"Publisher", item_.nonexacts.publisher}},
            {{"Edition", item_.nonexacts.edition}},
            {{"Year", year}},
            {std::nullopt},
            {{"Journal", item_.nonexacts.journal}},
            {{"Extension", item_.exacts.extension}},
            {std::nullopt},
            {{"Size", size}}, // TODO: print as red if very large
            {{"Pages", pages}},
            {{"Volume", volume}},
            {{"Number", number}},
            {{"Mirrors", uris}},
            {{"Source", item_.misc.origin_plugin}},
            {{"ISBNs", isbns}},
        };

        /* Find the longest string, and create an indent length by adding 4. */
        // clang-format off
        const auto indent = 4 + std::max_element(cbegin(v), cend(v), [](const auto &a, const auto &b) -> bool {
            /* Account for field separators. */
            if (!a.has_value())
                return true;
            if (!b.has_value())
                return false;

            return a->first.length() < b->first.length();
        })->value().first.length();
        // clang-format on

        /* Align all fields to calculated indent */
        int y = 1;
        for (const auto &p : v) {
            /* Separate field "categories" with an empty line */
            if (!p.has_value()) {
                y++;
                continue;
            }

            print(0, y, p->first + ':', attribute::bold);

            /* Split msg into n substrings of length get_width() - x. */
            std::vector<std::string> substrings;
            auto x = indent;
            for (size_t i = 0; i < p->second.get().length(); i += get_width() - x) {
                substrings.push_back(p->second.get().substr(i, get_width() - x));
            }

            for (const auto str : substrings) {
                print(indent, y++, str);
            }
        }
    }

} // namespace bookwyrm::tui::screen
