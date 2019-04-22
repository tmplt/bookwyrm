#include "screens/log.hpp"
#include <fmt/format.h>
#include "../../string.hpp"

namespace bookwyrm::tui::screen {

    log::log(const core::log_level wanted_level, std::function<bool(void)> &&predicate)
        : base(default_padding_top, default_padding_bot, default_padding_left, default_padding_right),
          wanted_level_(wanted_level), is_log_focused_(predicate)
    {
    }

    void log::maybe_update_detached(std::function<void()> &&fun)
    {
        if (detached_at_.has_value()) {
            /*
             * The entries_ container will after a while need to resize.
             * This may invalidate any iterators to an element within (e.g. detached_at_).
             */
            const auto dist = std::distance(entries_.crend(), detached_at_.value());
            fun();
            detached_at_ = entries_.crend() + dist; // `dist` is negative
        } else {
            fun();
        }
    }

    void log::mark_read()
    {
        maybe_update_detached([&]() {
            entries_.insert(entries_.end(), unread_entries_.begin(), unread_entries_.end());
            unread_entries_.clear();
            return;
        });
    }

    void log::paint()
    {
        erase();

        /* Figure out how many entries we can fit on screen; return staring point. */
        entry_ri entry = std::invoke([this]() {
            const auto last_entry = detached_at_.value_or(crbegin(entries_));
            int lines = get_height();

            return last_entry - 1 + std::count_if(last_entry, crend(entries_), [this, &lines](const auto e) {
                       lines -= std::ceil(static_cast<double>(e.second.length()) / get_width());
                       return lines >= 0;
                   });
        });

        int y = 0;
        while (entry != entries_.crbegin() - 1) {
            print_entry(y, entry--);
            y++;
        }

        refresh();
    }

    void log::print_entry(int &y, const entry_ri entry)
    {
        int x = 0;

        /*
         * First up, split the log level from the message, and print the
         * level in a fitting colour.
         */
        const auto[lvl, msg] = split_at_first(entry->second, ":");
        print(x, y, lvl, to_colour(entry->first));
        x += lvl.length();

        /* Split msg into n substrings of length get_width() - x. */
        std::vector<std::string> substrings;
        for (size_t i = 0; i < msg.length(); i += get_width() - x) {
            substrings.push_back(msg.substr(i, get_width() - x));
        }

        for (const auto str : substrings) {
            print(x, y++, str);
            x = 0;
        }

        y--;
        return;
    }

    std::string log::footer_info() const
    {
        return fmt::format("Log entries: {}; attached to tail: {}", entries_.size(), !detached_at_.has_value());
    }

    std::string log::controls_legacy() const { return "[j/k d/u]Navigation [SPACE]attach/detach"; }

    int log::scrollpercent() const
    {
        return ratio(std::distance(detached_at_.value_or(entries_.crbegin()), entries_.crend()), entries_.size());
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

        maybe_update_detached([&]() {
            emplace_entry(entries_, level, msg);
            return;
        });
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
