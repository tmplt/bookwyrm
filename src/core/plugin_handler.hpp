#pragma once

#include <unistd.h>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <unordered_set>
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
    explicit plugin_handler(const item &&wanted)
        : wanted_(wanted) {}

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
    std::unordered_set<core::item>& results()
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

    py::scoped_interpreter interp;
    std::unique_ptr<py::gil_scoped_release> nogil;

    const core::item wanted_;

    /* Somewhere to store our found items. */
    std::unordered_set<core::item> items_;

    /* A lock for when multiple threads want to add an item. */
    std::mutex items_mutex_;

    /* The same Python modules, but now running! */
    vector<std::thread> threads_;

    std::weak_ptr<frontend> frontend_;

    vector<py::module> plugins_;
};

/* ns bookwyrm::core */
}
