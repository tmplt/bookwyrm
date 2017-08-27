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
#include <mutex>

#include <spdlog/sinks/sink.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/logger.h>

#include "common.hpp"
#include "screens/log.hpp"

namespace spdlog::custom {

/*
 * A sink which prints level::err and above to stdcerr.
 * All other levels are considered not-errors, so they are
 * printed to stdout, alike the inherited sink.
 *
 * NOTE: if thread safety is needed, protect with a mutex.
 */
class bookwyrm_sink : public spdlog::sinks::sink {
public:
    bookwyrm_sink(bool &store_in_buffer)
        : log_to_screen_(store_in_buffer) {}

    void log(const details::log_msg &msg) override;
    void flush();

    void set_log_screen(std::shared_ptr<screen::log> screen)
    {
        log_screen_ = screen;
    }

private:
    const bool &log_to_screen_;
    std::mutex write_mutex_;
    std::shared_ptr<screen::log> log_screen_;
};

/* ns spdlog::custom */
}

namespace logger {

class bookwyrm_logger : public spdlog::logger {
public:
    explicit bookwyrm_logger(string name, std::shared_ptr<spdlog::custom::bookwyrm_sink> sink)
        : spdlog::logger(name, sink), sink_(sink)
    {

    }

    void set_log_screen(std::shared_ptr<screen::log> screen)
    {
        sink_->set_log_screen(screen);
    }

private:
    std::shared_ptr<spdlog::custom::bookwyrm_sink> sink_;

};

/* Create the logger. */
std::shared_ptr<bookwyrm_logger> create(std::string &&name, bool &tui_up);

}

using logger_t = std::shared_ptr<logger::bookwyrm_logger>;
