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

#include <spdlog/details/log_msg.h>
#include <spdlog/logger.h>
#include <spdlog/common.h>

#include "components/logger.hpp"

namespace spdlog::custom {

void split_sink::log(const details::log_msg &msg)
{
    if (msg.level <= spdlog::level::warn)
        std::cout << msg.formatted.str();
    else
        std::cerr << msg.formatted.str();
}

void split_sink::flush()
{
    std::cout << std::flush;
    std::cerr << std::flush;
}

/* ns spdlog::custom */
}

std::shared_ptr<spdlog::logger> logger::create(std::string &&name)
{
    auto sink = std::make_shared<spdlog::custom::split_sink>();
    auto logger = std::make_shared<spdlog::logger>(std::forward<std::string>(name),
            std::move(sink));

    return logger;
}

