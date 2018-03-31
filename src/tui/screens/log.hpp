#pragma once

#include <optional>

#include "plugin_handler.hpp"
#include "screens/base.hpp"

namespace bookwyrm::screen {

class log : public base {
public:
    explicit log();

    void paint() override;
    void toggle_action() override;
    void move(move_direction dir) override;
    std::string footer_info() const override;
    int scrollpercent() const override;

    std::string controls_legacy() const override
    {
        return "[j/k d/u]Navigation [SPACE]attach/detach";
    }

    void log_entry(core::log_level level, std::string msg);

private:
    using entry_t = std::pair<const core::log_level, const std::string>;
    std::vector<entry_t> entries_;
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
