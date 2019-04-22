#include "screens/log.hpp"
#include <fmt/format.h>
#include "../../string.hpp"

namespace bookwyrm::tui::screen {

    log::log(const core::log_level wanted_level, std::function<bool(void)> &&predicate)
        : base(default_padding_top, default_padding_bot, default_padding_left, default_padding_right),
          wanted_level_(wanted_level), is_log_focused_(predicate)
    {
    }

    void log::mark_read()
    {
        entries_.insert(entries_.end(), unread_entries_.begin(), unread_entries_.end());
        unread_entries_.clear();
    }

    void log::paint()
    {
        erase();

        const auto capacity = [this](auto e) -> int {
            // TODO: use std::count_if with reverse iterators
            int capacity = 0;
            for (int lines = get_height(); e != crend(entries_) && lines > 0; e++, capacity++) {
                lines -= std::ceil(static_cast<double>(e->second.length()) / get_width());
            }

            return capacity;
        };

        /*
         * Starting the counting from the latest entry,
         * how many entries back can we fit on screen?
         */
        const auto last_entry = detached_at_.value_or(crbegin(entries_));
        auto entry = last_entry + capacity(last_entry) - 1;

        int y = 0;
        while (entry != entries_.crbegin() - 1) {
            print_entry(y, entry--);
            y++;
        }

        refresh();
    }

    void log::print_entry(int &y, const log_pp entry)
    {
        int x = 0;

        /*
         * First up, split the log level from the message, and print the
         * level in a fitting colour.
         */
        const auto[lvl, msg] = split_at_first(entry->second, ":");
        print(x, y, lvl, to_colour(entry->first));
        x += static_cast<int>(lvl.length());

        /*
         * Next up, the actual message. If the whole message doesn't fit on one line
         * we want to split it across multiple lines. But course, if one word is
         * longer than the line itself (e.g. a long path), we'll just split it where
         * the line ends.
         */
        for (auto word : split_string(msg)) {
            if (int remain = get_width() - 1 - x; static_cast<int>(word.length()) + 1 > remain) {
                /* The word doesn't fit on the rest of the line. */

                /* 3 is an arbitrary divisor, but we use it so that only very long words
                 * are split. */
                if (static_cast<int>(word.length()) > get_width() / 3) {
                    while (static_cast<int>(word.length()) > remain) {
                        print(x, y++, " " + word.substr(0, remain));
                        word = word.substr(remain);
                        x = 0;
                        remain = get_width() - 1;
                    }
                } else {
                    ++y;
                    x = 0;
                }
            }

            print(x, y, " " + word);
            x += word.length() + 1;
        }
    }

    std::string log::footer_info() const
    {
        return fmt::format("Log entries: {}; attached to tail: {}", entries_.size(), !detached_at_.has_value());
    }

    std::string log::controls_legacy() const { return "[j/k d/u]Navigation [SPACE]attach/detach"; }

    int log::scrollpercent() const
    {
        // TODO: inline as value_or
        if (!detached_at_.has_value())
            return 100;

        return ratio(std::distance(*detached_at_, entries_.crend()), entries_.size());
    }

    void log::log_entry(core::log_level level, std::string msg)
    {
        if (level < wanted_level_)
            return;

        /*
         * We might get some error from Python here, which contain a few newlines.
         * Log entries are a single line, so we strip the line breaks here.
         *
         * XXX: what about \r?
         */
        std::replace(msg.begin(), msg.end(), '\n', ' ');

        auto emplace_entry = [&](std::vector<core::log_pair> &v, core::log_level lvl, std::string msg) {
            std::string prefix = core::loglvl_to_string(lvl);
            v.emplace_back(lvl, prefix + ": " + msg);
        };

        /* Is the log screen focused? */
        if (!is_log_focused_()) {
            emplace_entry(unread_entries_, level, msg);
            return;
        }

        if (detached_at_.has_value()) {
            /*
             * The entries_ container will after a while need to resize.
             * This may invalidate any iterators to an element within (e.g. detached_at_).
             */
            const auto dist = std::distance(entries_.crend(), detached_at_.value());
            emplace_entry(entries_, level, msg);
            detached_at_ = entries_.crend() + dist; // `dist` is negative
        } else {
            emplace_entry(entries_, level, msg);
        }
    }

    std::optional<core::log_level> log::worst_unread() const
    {
        const auto worst = std::max_element(
            cbegin(unread_entries_), cend(unread_entries_), [](const auto &a, const auto &b) { return a.first < b.first; });

        if (worst == unread_entries_.cend()) {
            /* No unread entries */
            return std::nullopt;
        }

        return worst->first;
    }

    std::vector<core::log_pair> log::unread_logs() const { return std::move(unread_entries_); }

    void log::toggle_action()
    {
        /* Toggle log attachment. */

        if (detached_at_.has_value())
            detached_at_.reset();
        else
            detached_at_ = entries_.crbegin();
    }

    void log::move(move_direction dir)
    {
        if (!detached_at_.has_value())
            return;

        const bool at_first_entry = *detached_at_ == entries_.crend() - 1,
                   at_last_entry = *detached_at_ == entries_.crbegin();

        switch (dir) {
        case up:
            if (at_first_entry)
                return;
            (*detached_at_)++;
            break;
        case down:
            if (at_last_entry)
                return;
            (*detached_at_)--;
            break;
        case top:
            detached_at_ = entries_.crend() - 1;
            break;
        case bot:
            detached_at_ = entries_.crbegin();
            break;
        }
    }

} // namespace bookwyrm::tui::screen
