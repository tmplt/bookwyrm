#include "screens/item_details.hpp"
#include "../utils.hpp"

namespace bookwyrm::tui::screen {

item_details::item_details(const core::item &item, int padding_top)
    : base(padding_top, default_padding_bot, 0, 0), item_(item)
{

}

bool item_details::action(const int ch)
{
    (void)ch;

    /* No actions for this screen yet. */

    return false;
}

void item_details::paint()
{
    print_borders();
    print_details();
}

std::string item_details::footer_info() const
{
    return fmt::format("DEBUG: padding top: {}, height: {}", padding_top_, get_height());
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
    const std::string uris = utils::vector_to_string(item_.misc.uris);

    using pair = std::pair<std::string, std::reference_wrapper<const std::string>>;
    std::string authors = utils::vector_to_string(item_.nonexacts.authors);
    std::string year = std::to_string(item_.exacts.year);
    const std::vector<pair> v = {
        {"Title",     item_.nonexacts.title},
        {"Serie",     item_.nonexacts.series},
        {"Authors",   authors},
        {"Year",      year},
        {"Publisher", item_.nonexacts.publisher},
        {"Extension", item_.exacts.extension},
        {"URI",       uris},
        // include filesize here
        // and print it red if the item is gigabytes large
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
        print(len + 4, y++, p.second.get());
    }

    print(0, ++y, "Description:", attribute::bold);
    print_desc(++y, utils::lipsum(20));
}

void item_details::print_desc(int &y, std::string str)
{
    int x = 0;
    const auto words = utils::split_string(str);

    auto word_fits = [this, &x](const std::string &str) -> bool {
        return static_cast<size_t>(get_width()) - x > str.length();
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
        x += word->length() + 1;
    }
}

/* ns bookwyrm::tui::screen */
}
