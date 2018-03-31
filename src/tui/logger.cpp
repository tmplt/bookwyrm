#include <iostream>
#include <utility>

#include "logger.hpp"

namespace bookwyrm::tui {

logger::~logger()
{
    for (const auto& [lvl, msg] : buffer_)
        (lvl <= core::log_level::warn ? std::cout : std::cerr) << msg;
}

void logger::log(const core::log_level lvl, string msg)
{
    if (lvl < wanted_level_)
        return;

    std::lock_guard<std::mutex> guard(write_mutex_);

    if (tui_.expired()) {
        buffer_.emplace_back(lvl, msg);
    } else if (const auto tui = tui_.lock(); tui->is_log_focused()) {
        tui->log(lvl, msg);
    } else {
        buffer_.emplace_back(lvl, msg);

        /* If user is in the index view, get a notice about new logs. */
        tui->repaint_screens();
    }
}

bool logger::has_unread_logs() const
{
    return !buffer_.empty();
}

core::log_level logger::worst_unread() const
{
    if (!has_unread_logs())
        throw std::runtime_error("__func__: buffer is empty");

    const auto worst = std::max_element(cbegin(buffer_), cend(buffer_),
        [] (const buffer_pair &a, const buffer_pair &b) {
            return a.first < b.first;
        });

    return worst->first;
}

void logger::flush_to_screen()
{
    if (tui_.expired())
        throw std::runtime_error("__func__: called after tui termination");

    const auto tui = tui_.lock();
    for (const auto& [lvl, msg] : buffer_)
        tui->log(lvl, msg);

    buffer_.clear();
}

void logger::set_tui(std::shared_ptr<tui> tui)
{
    tui_ = tui;
}

/* ns bookwyrm */
}
