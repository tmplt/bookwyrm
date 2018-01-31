#include <iostream>
#include <utility>

#include <spdlog/logger.h>
#include <spdlog/common.h>
#include <spdlog/details/log_msg.h>

#include "logger.hpp"

namespace logger {

void bookwyrm_sink::log(const spdlog::details::log_msg &msg)
{
    std::lock_guard<std::mutex> guard(write_mutex_);

    if (const auto &fmt = msg.formatted.str(); tui_.expired()) {
        buffer_.emplace_back(msg.level, fmt);
    } else if (const auto tui = tui_.lock(); tui->is_log_focused()) {
        tui->log(msg.level, fmt);
    } else {
        buffer_.emplace_back(msg.level, fmt);

        /* If user is in the index view, get a notice about new logs. */
        tui->repaint_screens();
    }
}

bookwyrm_sink::~bookwyrm_sink()
{
    for (const auto& [lvl, fmt] : buffer_)
        (lvl <= spdlog::level::warn ? std::cout : std::cerr) << fmt;
}

void bookwyrm_sink::flush()
{
    std::cout << std::flush;
    std::cerr << std::flush;
}

void bookwyrm_sink::flush_to_screen()
{
    const auto tui = tui_.lock();

    for (const auto& [lvl, fmt] : buffer_)
        tui->log(lvl, fmt);

    buffer_.clear();
}

spdlog::level::level_enum bookwyrm_sink::worst_unread() const
{
    const auto worst = std::max_element(cbegin(buffer_), cend(buffer_),
        [] (const buffer_pair &a, const buffer_pair &b) {
            return a.first < b.first;
        });

    return worst->first;
}

/* ns logger */
}

std::shared_ptr<logger::bookwyrm_logger> logger::create(std::string &&name)
{
    auto sink = std::make_unique<logger::bookwyrm_sink>();
    auto logger = std::make_shared<logger::bookwyrm_logger>(std::forward<std::string>(name),
            std::move(sink));

    return logger;
}
