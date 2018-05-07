#pragma once

#include <unistd.h>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <set>
#include <experimental/filesystem>

#include "item.hpp"
#include "python.hpp"
#include "hash.hpp"

namespace fs = std::experimental::filesystem;

namespace bookwyrm::core {

enum class log_level {
    trace    = 0,
    debug    = 1,
    info     = 2,
    warn     = 3,
    err      = 4,
    critical = 5,
    off      = 6
};

class frontend {
public:

    /* Updates the frontend after more items have been found. */
    virtual void update() = 0;

    /* Log something to the frontend with a fitting level. */
    virtual void log(const log_level level, const std::string message) = 0;
};

class __attribute__ ((visibility("hidden"))) plugin_handler {
public:
    explicit plugin_handler(const item &&wanted, bool debug)
        : wanted_(wanted), debug_(debug) {}

    /*
     * Explicitly delete the copy-constructor.
     * Doing this allows us to run each python
     * module in its own thread.
     *
     * Why we have to do this might be because threads_ is considered
     * copy-constructible, and when passing this to the Python module,
     * a copy is wanted instead of a reference.
     */
    explicit plugin_handler(const plugin_handler&) = delete;
    ~plugin_handler();

    /* Finds and loads all valid plugins. */
    void load_plugins();

    /* Start a std::thread for each valid plugin found. */
    void async_search();

    /* Try to add a found item, and then update the set frontend. */
    void add_item(std::tuple<core::nonexacts_t, core::exacts_t, core::misc_t> item_comps);

    void log(log_level lvl, std::string msg);

    // TODO: return const and make const
    std::set<core::item>& results()
    {
        return items_;
    }

    /* What frontend do we want to notify on updates? */
    void set_frontend(std::shared_ptr<frontend> fe)
    {
        frontend_ = fe;
    }

private:
    static inline bool readable_file(const fs::path &path)
    {
        return fs::is_regular_file(path) && access(path.c_str(), R_OK) == 0;
    }

    /* The item to propagate to all plugins. */
    const core::item wanted_;

    /* Should debug scripts be loaded? */
    const bool debug_;

    /* Somewhere to store our found items. */
    std::set<core::item> items_;

    /* A lock for when multiple threads want to add an item. */
    std::mutex items_mutex_;

    /*
     * Buffer log entries until a frontend is available.
     * This could be ditched if we enforce set_frontend() before load_plugins().
     */
    using buffer_pair = std::pair<const log_level, const std::string>;
    std::vector<buffer_pair> buffer_;
    std::weak_ptr<frontend> frontend_;

    /* Python-specific; do not change the order of this. */
    py::scoped_interpreter interp;
    vector<std::thread> threads_;
    vector<py::module> plugins_;
    std::unique_ptr<py::gil_scoped_release> nogil;
};

/* ns bookwyrm::core */
}
