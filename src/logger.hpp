#pragma once

#include <memory>
#include <ostream>
#include <mutex>

#include "common.hpp"
#include "tui.hpp"

namespace bookwyrm {

/* Circular dependency guard. */
class tui;

class logger {
public:
    // XXX: perhaps we should init the logger with the tui, instead?
    // after all, we only ever print to it when the TUI is up or when it's terminating.

    /* Flush buffer to std{out,err} */
    ~logger();

    // XXX: make this private?
    void log(const core::log_level lvl, string msg);

    // XXX: Can this somehow be done with templates?
    void trace(const string msg)
    {
        log(core::log_level::trace, "trace: " + msg);
    }
    void debug(const string msg)
    {
        log(core::log_level::debug, "debug: " + msg);
    }
    void info(const string msg)
    {
        log(core::log_level::info, "info: " + msg);
    }
    void warn(const string msg)
    {
        log(core::log_level::warn, "warning: " + msg);
    }
    void err(const string msg)
    {
        log(core::log_level::err, "error: " + msg);
    }
    void critical(const string msg)
    {
        log(core::log_level::critical, "critical: " + msg);
    }

    void set_level(const core::log_level lvl)
    {
        wanted_level_ = lvl;
    }

    bool has_unread_logs() const;
    core::log_level worst_unread() const;
    void flush_to_screen();
    void set_tui(std::shared_ptr<tui> tui);

private:
    core::log_level wanted_level_ = core::log_level::off;
    using buffer_pair = std::pair<const core::log_level, const string>;
    vector<buffer_pair> buffer_;
    std::mutex write_mutex_;

    std::weak_ptr<tui> tui_;
};

/* ns bookwyrm */
}

using logger_t = std::shared_ptr<bookwyrm::logger>;
