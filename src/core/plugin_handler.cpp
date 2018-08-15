#include <system_error>
#include <cerrno>
#include <cstdlib>
#include <array>

#include <fmt/format.h>

#include "prefix.hpp"
#include "python.hpp"
#include "plugin_handler.hpp"

using namespace bookwyrm::core;

void plugin_handler::load_plugins()
{
    /*
     * Append the plugin paths to Python's sys.path,
     * allowing them to be imported.
     */
    auto sys_path = py::reinterpret_borrow<py::list>(py::module::import("sys").attr("path"));
    for (auto &p : options_.plugin_paths)
        sys_path.append(p.string().c_str());

#if DEBUG
    /* And add the path to where pybookwyrm.so is available. */
    sys_path.append(options_.library_path.c_str());
    log(log_level::debug, fmt::format("coercing CPython to look for pybookwyrm in {}", options_.library_path));
#endif

    for (auto &path : options_.plugin_paths)
        log(log_level::debug, fmt::format("looking for scripts in {}", path.string()));

    /*
     * Find all Python modules and populate the
     * list of plugins by loading them.
     *
     * The first occurance of a module will be imported,
     * the latter ones will be ignored by Python. So we
     * either need to prepend the paths to sys.path, or
     * make sure that we don't clash with the many module
     * names in Python.
     */
    vector<py::module> plugins;
    for (const auto &plugin_path : options_.plugin_paths) {
        for (const fs::path &p : fs::directory_iterator(plugin_path)) {
            if (p.extension() != ".py") continue;
            /* Only load debug-* plugins if --debug is passed. */
            if (!debug_ && p.stem().string().rfind("debug-", 0) == 0) continue;

            if (!readable_file(p)) {
                log(log_level::err, fmt::format("can't load module '{}': not a regular file or unreadable"
                        "; ignoring...", p.string()));
                continue;
            }

            try {
                string module = p.stem();
                plugins.emplace_back(py::module::import(module.c_str()));
                log(log_level::debug, fmt::format("loaded module '{}'.", module));
            } catch (const py::error_already_set &err) {
                log(log_level::err, fmt::format("{}; ignoring...", err.what()));
            }
        }
    }

    if (plugins.empty())
        throw std::runtime_error("couldn't find any valid plugin scripts");

    plugins_ = std::move(plugins);
}

plugin_handler::~plugin_handler()
{
    /* Flush log entries. */
    // TODO: print log level before msg.
    for (const auto& [lvl, msg] : buffer_) {
        if (!debug_ && lvl <= log_level::debug) continue;
        (lvl <= log_level::warn ? std::cout : std::cerr)
            << loglvl_to_string(lvl) + ": " + msg << "\n";
    }

    for (auto &t : threads_)
        t.detach();

    frontend_.reset();
}

void plugin_handler::async_search()
{
    assert(!plugins_.empty());

    /* Ensure pybind internals are initialized. */
    py::get_shared_data("");

    log(log_level::debug, fmt::format("seaching with an accuracy of {}%", options_.fuzzy_threshold));

    /* Thread-safe lambda that notifies frontend about terminating plugin. */
    auto decrement_running_plugins = [this]() {
        running_plugins_--;
        if (auto fe = frontend_.lock(); fe)
            fe->update();
    };

    running_plugins_ = plugins_.size();

    for (py::module m : plugins_) {
        log(log_level::debug, fmt::format("running module '{}'", m.attr("__name__").cast<string>()));
        threads_.emplace_back([m, decrement_running_plugins, this]() mutable {
            /* Required whenever we need to run anything Python. */
            auto gil = std::make_unique<py::gil_scoped_acquire>();

            const string name = m.attr("__name__").cast<string>();

            /*
             * We have to go manual here. Normally, when unwinding on pthread exit,
             * Python operations may be performed without holding the GIL, leading to a segfault.
             *
             * This fix may only work on Linux, since abi::__forced_unwind is an implementation detail.
             */
            py::object func;
            py::tuple args = py::make_tuple(wanted_, this);

            try {
                /* Run the module's find function with the wanted item as argument. */
                func = m.attr("find");
                m.release().dec_ref();
                PyObject* retval = PyObject_Call(func.ptr(), args.ptr(), nullptr);

                /* Check if an exception was thrown */
                if (retval == nullptr) {
                    /* Coerce the error out from Python */
                    PyObject *ptype = nullptr, *pvalue = nullptr, *ptraceback = nullptr;
                    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
                    PyObject *utf8 = PyObject_Repr(pvalue);
                    PyObject *pystr = PyUnicode_AsEncodedString(utf8, "utf-8", nullptr);
                    const char *errmsg = PyBytes_AS_STRING(pystr);

                    /* Decrement reference count of used objects */
                    Py_XDECREF(utf8);
                    Py_XDECREF(pystr);
                    Py_XDECREF(ptype);
                    Py_XDECREF(pvalue);
                    Py_XDECREF(ptraceback);

                    log(log_level::err, fmt::format("plugin '{}' exited non-successfully: {}", name, errmsg));
                }

                Py_XDECREF(retval);

            } catch (abi::__forced_unwind&) {
                /*
                 * Forced stack unwinding at thread exit —
                 * if Python is shutting down, don't clean up Python state.
                 */
                if (!Py_IsInitialized()) {
                    args.release();
                    func.release();
                    gil.release();
                }
                throw;
            } catch (const py::error_already_set &err) {
                log(log_level::err, fmt::format("plugin '{}' exited non-successfully: {}",
                    name, err.what()));
            }

            decrement_running_plugins();
            log(log_level::debug, fmt::format("exiting plugin '{}'", name));
        });
    }

    /*
     * We have called all Python code we need to from here,
     * so we release the GIL and let the modules do their job.
     */
    this->nogil = std::make_unique<py::gil_scoped_release>();
}

void plugin_handler::wait()
{
    /*
     * Wait until all plugins have finished
     * TODO: clean this up
     */
    while (running_plugins_ != 0)
        ;
}

void plugin_handler::add_item(std::tuple<nonexacts_t, exacts_t, misc_t> item_comps)
{
    const item item(item_comps);
    if (!item.matches(wanted_, options_.fuzzy_threshold) || item.misc.uris.size() == 0)
        return;

    std::lock_guard<std::mutex> guard(items_mutex_);

    bool inserted = false;
    std::tie(std::ignore, inserted) = items_.insert(item);

    if (auto fe = frontend_.lock(); fe && inserted)
        fe->update();
}

void plugin_handler::log(log_level lvl, string msg)
{
    if (frontend_.expired()) {
        buffer_.emplace_back(lvl, msg);
    }  else {
        if (auto fe = frontend_.lock(); fe)
            fe->log(lvl, msg);
    }
}

std::set<item>& plugin_handler::results()
{
    return items_;
}

void plugin_handler::set_frontend(std::shared_ptr<frontend> fe)
{
    frontend_ = fe;

    /* Propegate the log buffer. */
    for (const auto& [lvl, msg] : buffer_)
        fe->log(lvl, msg);

    buffer_.clear();
}

const std::atomic<int>& plugin_handler::running_plugins() const
{
    return running_plugins_;
}

bool plugin_handler::readable_file(const fs::path &path)
{
    return fs::is_regular_file(path) && access(path.c_str(), R_OK) == 0;
}
