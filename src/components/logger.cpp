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

namespace spdlog::custom {

void split_sink::log(const details::log_msg &msg)
{
    const auto &str = msg.formatted.str();
    auto &out = msg.level <= spdlog::level::warn ? std::cout : std::cerr;

    std::lock_guard<std::mutex> guard(write_mutex_);

    if (store_in_buffer_)
        buffer_.emplace_back(out, str);
    else
        out << str;
}

void split_sink::flush()
{
    std::cout << std::flush;
    std::cerr << std::flush;
}

split_sink::~split_sink()
{
    for (auto& [out, msg] : buffer_)
        out.get() << msg;
}

/* ns spdlog::custom */
}

std::shared_ptr<spdlog::logger> logger::create(std::string &&name, bool &tui_up)
{
    auto sink = std::make_shared<spdlog::custom::split_sink>(tui_up);
    auto logger = std::make_shared<spdlog::logger>(std::forward<std::string>(name),
            std::move(sink));

    return logger;
}
