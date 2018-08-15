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

inline string loglvl_to_string(log_level lvl)
{
    vector<string> strings = {{
        "trace",
        "debug",
        "info",
        "warn",
        "error",
        "critical",
        "off",
    }};

    return strings[static_cast<int>(lvl)];
}

struct options {
    vector<fs::path> plugin_paths;
    string library_path;
    unsigned int fuzzy_threshold = 75;
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
    explicit plugin_handler(const item &&wanted, bool debug, const options options)
        : wanted_(wanted), debug_(debug), options_(options) {}

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

    /**
     * @brief Find and load all plugins
     */
    void load_plugins();

    /**
     * @brief Start searching with a dedicated thread for each found plugin
     * @warning Should be called after the \ref plugin_handler::load_plugins function
     */
    void async_search();

    void wait();

    /* Try to add a found item, and then update the set frontend. */

    /**
     * @brief Try to add a found item, and the update the set frontend.
     * @param item_comps Item components passed from CPython via Pybind11
     * @warning Should be called after the \ref plugin_handler::set_frontend function
     */
    void add_item(std::tuple<core::nonexacts_t, core::exacts_t, core::misc_t> item_comps);

    void log(log_level lvl, std::string msg);

    // TODO: return const and make const
    std::set<core::item>& results();

    /**
     * @brief Set the frontend that we want to notify on updates
     * @param fe The frontend that we want to notify
     */
    void set_frontend(std::shared_ptr<frontend> fe);

    /**
     * @brief Return how many plugins are still searching
     */
    const std::atomic<int>& running_plugins() const;

private:
    static bool readable_file(const fs::path &path);

    /* The item to propagate to all plugins. */
    const core::item wanted_;

    /* Should debug scripts be loaded? */
    const bool debug_;

    const options options_;

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

    std::atomic<int> running_plugins_{0};

    /* Python-specific; do not change the order of this. */
    py::scoped_interpreter interp;
    vector<std::thread> threads_;
    vector<py::module> plugins_;
    std::unique_ptr<py::gil_scoped_release> nogil;
};

/* ns bookwyrm::core */
}
