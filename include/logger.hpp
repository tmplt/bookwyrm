#pragma once

#include <memory>
#include <ostream>
#include <mutex>

#include <spdlog/sinks/sink.h>
#include <spdlog/details/log_msg.h>
#include <spdlog/logger.h>

#include "common.hpp"
#include "tui.hpp"

/* Circular dependency guard. */
namespace bookwyrm { class tui; }

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

    void set_tui(std::shared_ptr<bookwyrm::tui> tui)
    {
        tui_ = tui;
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

    std::weak_ptr<bookwyrm::tui> tui_;
};

class bookwyrm_logger : public spdlog::logger {
public:
    explicit bookwyrm_logger(string name, std::shared_ptr<bookwyrm_sink> sink)
        : spdlog::logger(name, sink), sink_(sink) {}

    void set_tui(std::shared_ptr<bookwyrm::tui> butler)
    {
        sink_->set_tui(butler);
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
