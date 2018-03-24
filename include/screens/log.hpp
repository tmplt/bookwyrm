#pragma once

#include <optional>

#include <spdlog/details/log_msg.h>

#include "screens/base.hpp"

namespace bookwyrm::screen {

class log : public base {
public:
    explicit log();

    void paint() override;
    void toggle_action() override;
    void move(move_direction dir) override;
    string footer_info() const override;
    int scrollpercent() const override;

    string controls_legacy() const override
    {
        return "[j/k d/u]Navigation [SPACE]attach/detach";
    }

    void log_entry(spdlog::level::level_enum level, string msg);

private:
    using entry_t = std::pair<spdlog::level::level_enum, const string>;
    vector<entry_t> entries_;
    using entry_tp = decltype(entries_.cbegin());

    std::optional<entry_tp> detached_at_;

    void print_entry(int &y, const entry_tp entry);

    /*
     * Returns the amount of entries in entries_ (starting at the last entry)
     * that can fit on screen.
     */
    size_t capacity(entry_tp entry) const;
};

/* ns screen */
}
