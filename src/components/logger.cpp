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

#include "components/logger.hpp"

namespace logger {

void bookwyrm_sink::log(const spdlog::details::log_msg &msg)
{
    const auto &str = msg.formatted.str();
    auto &out = msg.level <= spdlog::level::warn ? std::cout : std::cerr;

    std::lock_guard<std::mutex> guard(write_mutex_);

    if (log_to_screen_)
        screen_butler_->log_entry(msg.level, str);
    else
        out << str;
}

void bookwyrm_sink::flush()
{
    std::cout << std::flush;
    std::cerr << std::flush;
}

/* ns logger */
}

std::shared_ptr<logger::bookwyrm_logger> logger::create(std::string &&name, bool &tui_up)
{
    auto sink = std::make_shared<logger::bookwyrm_sink>(tui_up);
    auto logger = std::make_shared<logger::bookwyrm_logger>(std::forward<std::string>(name),
            std::move(sink));

    return logger;
}
