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

#include "screens/log.hpp"
#include "utils.hpp"

namespace screen {

log::log()
    : base(default_padding_top, default_padding_bot, default_padding_left, 0)
{

}

bool log::action(const key &key, const uint32_t &ch)
{
    /* stub */
    (void)key;
    (void)ch;

    return false;
}

void log::update()
{
    /*
     * Starting the counting from the latest entry,
     * how many entries back can we fit on screen?
     */
    auto entry = entries_.cend() - capacity();

    int y = 0;
    while (entry != entries_.cend()) {
        print_entry(y, entry++);
        y++;
    }
}

void log::print_entry(int &y, const entry_tp entry)
{
    int x = 0;

    const auto [lvl, rest] = utils::split_at_first(entry->second, ":");
    mvprintw(x, y, lvl, utils::to_colour(entry->first));
    x += lvl.length();

    for (const auto &word : utils::split_string(rest)) {
        if (word.length() + 1 > get_width() - 1 - x) {
            ++y;
            x = lvl.length();
        }

        mvprintw(x, y, " " + word);
        x += word.length() + 1;
    }
}

void log::on_resize()
{
    /* stub */
}

string log::footer_info() const
{
    /* stub */
    return fmt::format("You're in the log now. Entries: {}, Capacity: {}",
            entries_.size(), capacity());
}

int log::scrollperc() const
{
    /* stub */
    return 42;
}

void log::log_entry(spdlog::level::level_enum level, string entry)
{
    /*
     * We might get some error from Python here, which contain a few newlines.
     * Log entries are a single line, so we strip the line breaks here.
     *
     * Note: what about \r?
     */
    std::replace(entry.begin(), entry.end(), '\n', ' ');

    entries_.emplace_back(level, entry);
}

size_t log::capacity() const
{
    auto remain = get_height();
    int capacity = 0;
    auto entry = entries_.cbegin() + entry_offset_;

    const auto entry_height = [this] (const auto e) -> int {
        /* return std::max<size_t>(e->second.size() / get_width(), 1); */
        // lazy string eval?
        (void)e;
        return 1;
    };

    while (entry++ != entries_.cend() && remain > 0) {
        remain -= entry_height(entry);
        capacity++;
    }

    return capacity;
}

/* ns screen */
}
