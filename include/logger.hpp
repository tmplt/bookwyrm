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
#include "screen_butler.hpp"

/* Circular dependency guard. */
namespace butler { class screen_butler; }

namespace logger {

/*
 * A sink which stores all logs in a buffer. Can be flushed to a screen butler
 * on command. If buffer_ is non-empty on object destruction, buffer content is
 * written to std{out,err}.
 */
class bookwyrm_sink : public spdlog::sinks::sink {
public:
    ~bookwyrm_sink();

    void log(const spdlog::details::log_msg &msg) override;
    void flush() override;

    void set_screen_butler(std::shared_ptr<butler::screen_butler> butler)
    {
        screen_butler_ = butler;
    }

    /* Flush all unseen logs (content of buffer_) to the log screen. */
    void flush_to_screen();

    bool has_unread_logs() const
    {
        return !buffer_.empty();
    }

    spdlog::level::level_enum worst_unread() const;

private:
    using buffer_pair = std::pair<spdlog::level::level_enum, const string>;
    vector<buffer_pair> buffer_;
    std::mutex write_mutex_;

    std::weak_ptr<butler::screen_butler> screen_butler_;
};

class bookwyrm_logger : public spdlog::logger {
public:
    explicit bookwyrm_logger(string name, std::shared_ptr<bookwyrm_sink> sink)
        : spdlog::logger(name, sink), sink_(sink) {}

    void set_screen_butler(std::shared_ptr<butler::screen_butler> butler)
    {
        sink_->set_screen_butler(butler);
    }

    void flush_to_screen()
    {
        sink_->flush_to_screen();
    }

    bool has_unread_logs() const
    {
        return sink_->has_unread_logs();
    }

    spdlog::level::level_enum worst_unread() const
    {
        return sink_->worst_unread();
    }

private:
    std::shared_ptr<bookwyrm_sink> sink_;
};

/* Create the logger. */
std::shared_ptr<bookwyrm_logger> create(std::string &&name);

}

using logger_t = std::shared_ptr<logger::bookwyrm_logger>;
