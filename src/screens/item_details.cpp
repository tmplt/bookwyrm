/*
 * Copyright (C) 2017 Tmplt <tmplt@dragons.rocks>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "screens/item_details.hpp"

namespace screen {

item_details::item_details(const bookwyrm::item &item, int padding_top)
    : base(padding_top, default_padding_bot, 0, 0), item_(item)
{

}

bool item_details::action(const key &key, const uint32_t &ch)
{
    (void)key;
    (void)ch;

    return false;
}

void item_details::paint()
{
    print_borders();
    print_details();
}

string item_details::footer_info() const
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
        for (size_t x = 0; x < get_width(); x++)
            change_cell(x, y, rune::single::em_dash);
    };

    print_line(0);
    print_line(get_height() - 1);
}

void item_details::print_details()
{
    const string uris = utils::vector_to_string(item_.misc.uris);

    using pair = std::pair<string, std::reference_wrapper<const string>>;
    const vector<pair> v = {
        {"Title",     item_.nonexacts.title},
        {"Serie",     item_.nonexacts.series},
        {"Authors",   item_.nonexacts.authors_str},
        {"Year",      item_.exacts.year_str},
        {"Publisher", item_.nonexacts.publisher},
        {"Format",    item_.exacts.format},
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
        wprint(0, y, p.first + ':', attribute::bold);
        wprint(len + 4, y++, p.second.get());
    }

    wprint(0, ++y, "Description:", attribute::bold);
    print_desc(++y, utils::lipsum(20));
}

void item_details::print_desc(int &y, string str)
{
    int x = 0;
    const auto words = utils::split_string(str);

    auto word_fits = [this, &x](const string &str) -> bool {
        return static_cast<size_t>(get_width()) - x > str.length();
    };

    for (auto word = words.cbegin(); word != words.cend(); ++word) {
        if (!word_fits(*word)) {
            if (y + 1u == get_height() - 1) {
                /* No more lines to draw on; can't fit any more. */

                if (word != words.cend() - 1) {
                    /* We haven't printed the whole description yet. */

                    /*
                     * Make sure the dots are printed in the screen.
                     * Subtracts an additional 1 to overwrite the space
                     * from the last word.
                     */
                    wprint(word_fits("...") ? --x : x - 4, y, "...");
                }

                return;
            }

            ++y;
            x = 0;
        }

        wprint(x, y, *word + ' ');
        x += (*word).length() + 1;
    }
}

/* ns screen */
}
