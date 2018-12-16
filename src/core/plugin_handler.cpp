#include <array>
#include <cerrno>
#include <cstdlib>
#include <functional>
#include <system_error>

#include <fmt/format.h>

#include "../prefix.hpp"
#include "plugin_handler.hpp"
#include "python.hpp"

using namespace bookwyrm::core;

void plugin_handler::load_plugins()
{
    /*
     * Append the plugin paths to Python's sys.path,
     * allowing them to be imported.
     * Make sure that the plugin doesn't share name with a standard lib module.
     */
    auto sys_path = py::reinterpret_borrow<py::list>(py::module::import("sys").attr("path"));
    const auto standard_modules = std::invoke([]() {
        /* Get the names of all standard Python modules */
        std::vector<std::string> names;
        for (auto &handle : py::list(py::module::import("pkgutil").attr("iter_modules")())) {
            names.push_back(py::str(handle.attr("name")));
        }

        return names;
    });
    for (auto &path : options_.plugin_paths) {
        for (auto &plugin : fs::directory_iterator(path)) {
            if (!fs::is_regular_file(plugin)) {
                /* We'll only import regular files */
                continue;
            }

            if (std::find(standard_modules.cbegin(), standard_modules.cend(), plugin.path().stem().string()) !=
                standard_modules.cend()) {
                throw std::runtime_error(fmt::format("cannot load plugin that shares name with a standard "
                                                     "library module ({})",
                                                     plugin.path().string()));
            }
        }

        sys_path.append(path.string().c_str());
    }

#if DEBUG
    /* Add the path to where pybookwyrm.so is available. */
    sys_path.append(options_.library_path.c_str());
    log(log_level::debug, fmt::format("coercing CPython to look for pybookwyrm in {}", options_.library_path));
#endif

    /*
     * Triage fix for crash in detail::to_py_dict if no loaded plugin imports pybookwyrm.
     * TODO: do away with this, and link pybookwyrm instead.
     */
    std::ignore = py::module::import("pybookwyrm");

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
        for (const fs::path &path : fs::directory_iterator(plugin_path)) {

            /* Check if we can load the file */
            if (path.extension() != ".py") {
                log(log_level::debug, fmt::format("'{}' doesn't look like a Python module; ignoring...", path.string()));
                continue;
            }
            if (!readable_file(path)) {
                log(log_level::err,
                    fmt::format("can't load module '{}': not a regular file or unreadable"
                                "; ignoring...",
                                path.string()));
                continue;
            }

            /* Load the module */
            try {
                string module = path.stem();
                plugins.emplace_back(py::module::import(module.c_str()));
                log(log_level::debug, fmt::format("loaded module '{}'", module));
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
    /*
     * Flush log entries.
     * TODO: colour this output to match that of the log screen.
     */
    for (const auto & [ lvl, msg ] : buffer_) {
        if (!debug_ && lvl <= log_level::debug)
            continue;
        (lvl <= log_level::warn ? std::cout : std::cerr) << loglvl_to_string(lvl) + ": " + msg << "\n";
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

    log(log_level::debug, fmt::format("seaching with an accuracy of {}%", options_.accuracy));
    running_plugins_ = plugins_.size();

    /* Start running each loaded plugin in a seperate thread */
    for (py::module module : plugins_) {
        log(log_level::debug, fmt::format("running module '{}'", module.attr("__name__").cast<string>()));
        threads_.emplace_back(&plugin_handler::python_module_runner, this, module);
    }

    /*
     * We have called all Python code we need to from here.
     * We release the GIL and let the modules do their job.
     */
    this->nogil = std::make_unique<py::gil_scoped_release>();
}

void plugin_handler::python_module_runner(py::module module)
{
    /* Aqcuire the Global Interpreter Lock, required for running any Python code.
     */
    auto gil = std::make_unique<py::gil_scoped_acquire>();

    const string name = module.attr("__name__").cast<string>();

    /*
     * We have to go manual here. Normally, when unwinding on pthread exit,
     * Python operations may be performed without holding the GIL, leading to a
     * segfault. See <https://github.com/pybind/pybind11/issues/1360>.
     *
     * This fix may only work on Linux, since abi::__forced_unwind is an
     * implementation detail.
     */
    py::object func;
    py::tuple args;
    py::dict wanted = detail::to_py_dict(wanted_);

    try {
        /* Run the module's find-function with the wanted item, and bookwyrm
         * instance as argument. */
        func = module.attr("find");
        args = py::make_tuple(wanted, this);
        module.release().dec_ref();
        PyObject *retval = PyObject_Call(func.ptr(), args.ptr(), nullptr);

        /* Check if an exception was thrown */
        if (retval == nullptr) {
            /* Coerce the exception error message out from CPython */
            PyObject *ptype = nullptr, *pvalue = nullptr, *ptraceback = nullptr;
            PyErr_Fetch(&ptype, &pvalue, &ptraceback);
            PyObject *utf8 = PyObject_Repr(pvalue);
            PyObject *pystr = PyUnicode_AsEncodedString(utf8, "utf-8", nullptr);
            const char *errmsg = PyBytes_AS_STRING(pystr);

            /* Find where the error was thrown. */
            auto *traceback = (PyTracebackObject *)ptraceback;
            while (traceback->tb_next != nullptr)
                traceback = traceback->tb_next;
            const int lineno = traceback->tb_lineno;

            /* Decrement reference count of used objects */
            Py_XDECREF(utf8);
            Py_XDECREF(pystr);
            Py_XDECREF(ptype);
            Py_XDECREF(pvalue);
            Py_XDECREF(ptraceback);

            log(log_level::err, fmt::format("plugin '{}' exited non-successfully (line: {}): {}", name, lineno, errmsg));
        }

        Py_XDECREF(retval);

    } catch (abi::__forced_unwind &) {
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
        log(log_level::err, fmt::format("plugin '{}' exited non-successfully: {}", name, err.what()));
    } catch (const py::cast_error &err) {
        log(log_level::err,
            fmt::format("plugin '{}' did something wrong with types; "
                        "does the module import the required pybookwyrm module? "
                        "Details: {}",
                        name,
                        err.what()));
    }

    /* Propegate that this plugin is terminating */
    log(log_level::debug, fmt::format("exiting plugin '{}'", name));
    running_plugins_--;
    if (auto fe = frontend_.lock(); fe)
        fe->update();
}

#ifdef DEBUG
void plugin_handler::wait()
{
    /*
     * Wait until all plugins have finished
     * TODO: clean this up
     */
    while (running_plugins_ != 0)
        ;
}
#endif

void plugin_handler::add_item(py::dict dict)
{
    const item item(dict);
    log(log_level::debug, fmt::format("trying to add one new item with title '{}'...", item.nonexacts.title));
    if (!item.matches(wanted_, options_.accuracy) || item.misc.uris.size() == 0)
        return;

    std::lock_guard<std::mutex> guard(items_mutex_);

    bool inserted = false;
    std::tie(std::ignore, inserted) = items_.insert(item);

    if (inserted)
        log(log_level::debug, "added one new item");
    else
        log(log_level::debug, "ignored one too similar item");

    if (auto fe = frontend_.lock(); fe && inserted)
        fe->update();
}

void plugin_handler::log(log_level lvl, string msg)
{
    if (frontend_.expired()) {
        buffer_.emplace_back(lvl, msg);
    } else {
        if (auto fe = frontend_.lock(); fe)
            fe->log(lvl, msg);
    }
}

std::set<item> &plugin_handler::results()
{
    return items_;
}

void plugin_handler::set_frontend(std::shared_ptr<frontend> fe)
{
    frontend_ = fe;

    /* Propegate the log buffer, if any entries. */
    for (const auto & [ lvl, msg ] : buffer_)
        fe->log(lvl, msg);
    buffer_.clear();
}

const std::atomic<size_t> &plugin_handler::running_plugins() const
{
    return running_plugins_;
}

bool plugin_handler::readable_file(const fs::path &path)
{
    return fs::is_regular_file(path) && access(path.c_str(), R_OK) == 0;
}
