#include "screens/log.hpp"
#include "utils.hpp"

namespace bookwyrm::screen {

log::log()
    : base(default_padding_top, default_padding_bot, default_padding_left, default_padding_right)
{

}

void log::paint()
{
    /*
     * Ad-hoc for now; something is wrong with entry down below.
     * Removing this causes segfault when we have no entries.
     */
    if (entries_.size() == 0)
        return;

    /*
     * Starting the counting from the latest entry,
     * how many entries back can we fit on screen?
     */
    const auto start_entry = detached_at_.value_or(entries_.cend());
    auto entry = start_entry - capacity(start_entry) - 1;

    int y = 0;
    while (entry != entries_.cend()) {
        print_entry(y, entry++);
        y++;
    }
}

void log::print_entry(int &y, const entry_tp entry)
{
    int x = 0;

    /*
     * First up, split the log level from the message, and print the
     * level in a fitting colour.
     */
    const auto [lvl, msg] = utils::split_at_first(entry->second, ":");
    wprint(x, y, lvl, utils::to_colour(entry->first));
    x += lvl.length();

    /*
     * Next up, the actual message. If the whole message doesn't fit on one line
     * we want to split it across multiple lines. But course, if one word is longer
     * than the line itself (e.g. a long path), we'll just split it where the line ends.
     */
    for (auto word : utils::split_string(msg)) {
        if (size_t remain = get_width() - 1 - x; word.length() + 1 > remain) {
            /* The word doesn't fit on the rest of the line. */

            /* 3 is an arbitrary divisor, but we use it so that only very long words are split. */
            if (word.length() > static_cast<size_t>(get_width() / 3)) {
                while (word.length() > remain) {
                    wprint(x, y++, " " + word.substr(0, remain));
                    word = word.substr(remain);
                    x = 0;
                    remain = get_width() - 1;
                }
            } else {
                ++y;
                x = 0;
            }
        }

        wprint(x, y, " " + word);
        x += word.length() + 1;
    }
}

string log::footer_info() const
{
    /* stub */
    return fmt::format("You're in the log now. Entries: {}, Attached: {}",
            entries_.size(), !detached_at_.has_value());
}

int log::scrollpercent() const
{
    if (!detached_at_.has_value())
        return 100;

    return utils::ratio(std::distance(entries_.cbegin(), *detached_at_), entries_.size());
}

void log::log_entry(spdlog::level::level_enum level, string msg)
{
    /*
     * We might get some error from Python here, which contain a few newlines.
     * Log entries are a single line, so we strip the line breaks here.
     *
     * Note: what about \r?
     */
    std::replace(msg.begin(), msg.end(), '\n', ' ');

    /*
     * The entries_ container will after a while need to resize,
     * this invalidates any pointers to an element within (detached_at_).
     * So we must update that pointer here, if we indeed are detached.
     */
    if (detached_at_.has_value()) {
        const auto dist = std::distance(entries_.cbegin(), detached_at_.value());
        entries_.emplace_back(level, msg);
        detached_at_ = entries_.cbegin() + dist;
    } else {
        entries_.emplace_back(level, msg);
    }
}

size_t log::capacity(entry_tp entry) const
{
    size_t remain = get_height();
    int capacity = 0;
    entry--;  // We want to point at something that exists.

    const auto entry_height = [line_width=get_width()] (const auto e) -> size_t {
        return std::max<size_t>(std::ceil(e->second.length() / line_width), 1);
    };

    while (entry != entries_.cbegin() && remain > 0) {
        remain -= entry_height(entry--);
        if (remain > 0)
            capacity++;
    }

    return capacity;
}

void log::toggle_action()
{
    /* Toggle log attachment. */

    if (detached_at_.has_value())
        detached_at_.reset();
    else
        detached_at_ = entries_.cend();
}

void log::move(move_direction dir)
{
    if (!detached_at_.has_value())
        return;

    const bool at_first_entry = *detached_at_ == entries_.cbegin() + 1,
               at_last_entry  = *detached_at_ == entries_.cend() - 1;

    switch (dir) {
        case up:
            if (at_first_entry) return;
            (*detached_at_)--;
            break;
        case down:
            if (at_last_entry) return;
            (*detached_at_)++;
            break;
        case top:
            detached_at_ = entries_.cbegin() + 1;
            break;
        case bot:
            detached_at_ = entries_.cend() - 1;
            break;
    }
}

/* ns screen */
}
