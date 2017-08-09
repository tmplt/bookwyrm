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

#pragma once

#include <memory>
#include <ostream>

#include <spdlog/sinks/sink.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/logger.h>

#include "common.hpp"

namespace spdlog::custom {

/*
 * A sink which prints level::err and above to stdcerr.
 * All other levels are considered not-errors, so they are
 * printed to stdout, alike the inherited sink.
 *
 * NOTE: if thread safety is needed, protect with a mutex.
 */
class split_sink : public spdlog::sinks::sink {
public:
    split_sink(bool &tui_up)
        : tui_up_(tui_up) {}
    ~split_sink();

    void log(const details::log_msg &msg) override;
    void flush();

private:
    /* If the TUI is up, we store logs here until program termination. */
    using buffer_pair = std::pair<std::reference_wrapper<std::ostream>, string>;
    vector<buffer_pair> buffer_;
    const bool &tui_up_;
};

/* ns spdlog::custom */
}

namespace logger {

/* Create the logger. */
std::shared_ptr<spdlog::logger> create(std::string &&name, bool &tui_up);

}

using logger_t = std::shared_ptr<spdlog::logger>;
