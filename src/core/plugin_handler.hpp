#pragma once

#include <atomic>
#include <experimental/filesystem>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <unistd.h>

#include "hash.hpp"
#include "item.hpp"
#include "python.hpp"

namespace fs = std::experimental::filesystem;

namespace bookwyrm::core {

    enum class log_level { trace = 0, debug = 1, info = 2, warn = 3, err = 4, critical = 5, off = 6 };
    using log_pair = std::pair<const core::log_level, const std::string>;

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

        return strings[static_cast<size_t>(lvl)];
    }

    struct options {
        vector<fs::path> plugin_paths;
        string library_path;
        unsigned int accuracy = 75;
    };

    class frontend {
    public:
        virtual ~frontend() {}

        /* Updates the frontend after more items have been found. */
        virtual void update() = 0;

        /* Log something to the frontend with a fitting level. */
        virtual void log(const log_level level, const std::string message) = 0;
    };

    class backend {
    public:
        virtual ~backend() {}

        virtual size_t running_plugins() const = 0;

        /* Found items are immutable outside of backend. */
        virtual const std::set<core::item> &search_results() const = 0;
    };

    class __attribute__((visibility("hidden"))) plugin_handler : public backend {
    public:
        explicit plugin_handler(const item &&wanted, bool debug, const options options);

        /*
         * Explicitly delete the copy-constructor.
         * Doing this allows us to run each python
         * module in its own thread.
         *
         * Why we have to do this might be because threads_ is considered
         * copy-constructible, and when passing this to the Python module,
         * a copy is wanted instead of a reference.
         */
        explicit plugin_handler(const plugin_handler &) = delete;
        ~plugin_handler();

        /**
         * @brief Find and load all plugins
         */
        void load_plugins();

        /**
         * @brief Start searching with a dedicated thread for each found plugin
         * @warning Should be called after the \ref plugin_handler::load_plugins
         * function
         */
        void async_search();

        /**
         * @brief Copy loaded modules from instance.
         */
        vector<py::module> get_plugins();

#ifdef DEBUG
        /**
         * @brief Wait for all plugins to finish execution
         * @warning Should be called after the \ref plugin_handler::async_search
         * function
         */
        void wait();
#endif

        /**
         * @brief Wait until one item has been found or until no modules are running.
         */
        void wait_for_item();

        /**
         * @brief Try to add a found item, and the update the set frontend.
         * @param dict Python dictionary containing all item information
         * @warning Should be called after the \ref plugin_handler::set_frontend
         * function
         */
        void add_item(py::dict dict);

        /**
         * @brief Log an error message
         *
         * Logged message is propagated to the configured frontend, if any.
         * Any non-propagated logs are flushed to std{out,err} upon plugin_handler
         * destruction.
         */
        void log(log_level lvl, std::string msg);

        /**
         * @brief Return all found items
         *
         * TODO [doc] return const and make const
         */
        const std::set<core::item> &search_results() const;

        /**
         * @brief Set the frontend that we want to notify on updates
         * @param fe The frontend that we want to notify
         */
        void set_frontend(std::shared_ptr<frontend> fe);
        void clear_frontend();
        void clear_nogil() { delete this->nogil.release(); }

        /**
         * @brief Return how many plugins are still searching
         */
        size_t running_plugins() const { return running_plugins_.load(); }

        inline size_t items() { return items_.size(); }

    private:
        static bool readable_file(const fs::path &path);
        void python_module_runner(py::module module);

        /* The item to propagate to all plugins. */
        const core::item wanted_;

        /* Should debug logs be printed for the user? */
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

        std::atomic<size_t> running_plugins_{0};

        /* Python-specific; do not change the order of this. */
        py::scoped_interpreter interp;
        vector<std::thread> threads_;
        vector<py::module> plugins_;
        std::unique_ptr<py::gil_scoped_release> nogil;
    };

} // namespace bookwyrm::core
