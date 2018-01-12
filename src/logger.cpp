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

    if (const auto &fmt = msg.formatted.str(); screen_butler_.expired()) {
        buffer_.emplace_back(msg.level, fmt);
    } else if (const auto screen = screen_butler_.lock(); screen->is_log_focused()) {
        screen->log(msg.level, fmt);
    } else {
        buffer_.emplace_back(msg.level, fmt);

        /* If user is in the index view, get a notice about new logs. */
        screen->repaint_screens();
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
    const auto screen = screen_butler_.lock();

    for (const auto& [lvl, fmt] : buffer_)
        screen->log(lvl, fmt);

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
