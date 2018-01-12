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

#include <system_error>
#include <cerrno>
#include <cstdlib>
#include <array>
#include <experimental/filesystem>

#include <fmt/format.h>

#include "utils.hpp"
#include "python.hpp"
#include "plugin_handler.hpp"

namespace fs = std::experimental::filesystem;

namespace core {

void plugin_handler::load_plugins()
{
    vector<fs::path> plugin_paths;
#ifdef DEBUG
    /* Bookwyrm must be run from build/ in DEBUG mode. */
    plugin_paths = { fs::canonical(fs::path("../src/core/plugins")) };
#else
    /* TODO: look through /etc/bookwyrm/plugins/ also. */
    if (fs::path conf = std::getenv("XDG_CONFIG_HOME"); !conf.empty())
        plugin_paths.push_back(conf / "bookwyrm/plugins");
    else if (fs::path home = std::getenv("HOME"); !home.empty())
        plugin_paths.push_back(home / ".config/bookwyrm/plugins");
    else
        log(log_level::err, "couldn't find any plugin directories.");
#endif

    /*
     * Append the plugin paths to Python's sys.path,
     * allowing them to be imported.
     */
    auto sys_path = py::reinterpret_borrow<py::list>(py::module::import("sys").attr("path"));
    for (auto &p : plugin_paths)
        sys_path.append(p.string().c_str());

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
    for (const auto &plugin_path : plugin_paths) {
        for (const fs::path &p : fs::directory_iterator(plugin_path)) {
            if (p.extension() != ".py") continue;

            if (!utils::readable_file(p)) {
                log(log_level::err, fmt::format("can't load module '{}': not a regular file or unreadable"
                        "; ignoring...", p.string()));
                continue;
            }

            try {
                string module = p.stem();
                log(log_level::debug, fmt::format("loading module '{}'...", module));
                plugins.emplace_back(py::module::import(module.c_str()));
            } catch (const py::error_already_set &err) {
                log(log_level::err, fmt::format("{}; ignoring...", err.what()));
            }
        }
    }

    if (plugins.empty())
        throw std::runtime_error("couldn't find any valid plugin scripts");

    plugins_ = plugins;
}

plugin_handler::~plugin_handler()
{
    for (auto &t : threads_)
        t.detach();
}

void plugin_handler::async_search()
{
    for (const auto &m : plugins_) {
        threads_.emplace_back([&m, wanted = wanted_, instance = this]() {
            /* Required whenever we need to run anything Python. */
            py::gil_scoped_acquire gil;

            try {
                m.attr("find")(wanted, instance);
            } catch (const py::error_already_set &err) {
                instance->log(log_level::err, fmt::format("module '{}' did something wrong: {}; ignoring...",
                    m.attr("__name__").cast<string>(), err.what()));
            }
        });
    }

    /*
     * We have called all Python code we need to from here,
     * so we release the GIL and let the modules do their job.
     */
    this->nogil = std::make_unique<py::gil_scoped_release>();
}

void plugin_handler::add_item(std::tuple<nonexacts_t, exacts_t, misc_t> item_comps)
{
    item item(item_comps);
    if (!item.matches(wanted_) || item.misc.uris.size() == 0)
        return;

    std::lock_guard<std::mutex> guard(items_mutex_);

    items_.push_back(item);

    if (!frontend_.expired()) {
        const auto fe = frontend_.lock();
        fe->update();
    }
}

void plugin_handler::log(log_level lvl, string msg)
{
    if (!frontend_.expired()) {
        const auto fe = frontend_.lock();
        fe->log(lvl, msg);
    }
}

/* ns butler */
}
