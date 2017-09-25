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

#include "utils.hpp"
#include "python.hpp"
#include "components/script_butler.hpp"

namespace fs = std::experimental::filesystem;

namespace butler {

script_butler::script_butler(const bookwyrm::item &&wanted)
    : wanted_(wanted) {}

vector<pybind11::module> script_butler::load_sources()
{
    vector<fs::path> source_paths;
#ifdef DEBUG
    /* Bookwyrm must be run from build/ in DEBUG mode. */
    source_paths = { fs::canonical(fs::path("../src/sources")) };
#else
    /* TODO: look through /etc/bookwyrm/sources/ also. */
    if (fs::path conf = std::getenv("XDG_CONFIG_HOME"); !conf.empty())
        source_paths.push_back(conf / "bookwyrm/sources");
    else if (fs::path home = std::getenv("HOME"); !home.empty())
        source_paths.push_back(home / ".config/bookwyrm/sources");
    else
        logger_->error("couldn't find any source script directories.");
#endif

    /*
     * Append the source paths to Python's sys.path,
     * allowing them to be imported.
     */
    auto sys_path = py::reinterpret_borrow<py::list>(py::module::import("sys").attr("path"));
    for (auto &p : source_paths)
        sys_path.append(p.string().c_str());

    /*
     * Find all Python modules and populate the
     * list of sources by loading them.
     *
     * The first occurance of a module will be imported,
     * the latter ones will be ignored by Python. So we
     * either need to prepend the paths to sys.path, or
     * make sure that we don't clash with the many module
     * names in Python.
     */
    vector<py::module> sources;
    for (const auto &source_path : source_paths) {
        for (const fs::path &p : fs::directory_iterator(source_path)) {
            if (p.extension() != ".py") continue;

            if (!utils::readable_file(p)) {
                logger_->error("can't load module '{}': not a regular file or unreadable"
                        "; ignoring...", p.string());
                continue;
            }

            try {
                string module = p.stem();
                logger_->debug("loading module '{}'...", module);
                sources.emplace_back(py::module::import(module.c_str()));
            } catch (const py::error_already_set &err) {
                logger_->error("{}; ignoring...", err.what());
            }
        }
    }

    if (sources.empty())
        throw program_error("couldn't find any valid source scripts, terminating...");

    return sources;
}

script_butler::~script_butler()
{
    /*
     * If we want to terminate the program, we need to join the threads.
     * And if we want to join the threads, the Python scripts must stop (e.g. return).
     * Since C++ offers no way to terminate an std::thread before it's done, we
     * signal the Python scripts that they should return here.
     */
    destructing_ = true;
    py::gil_scoped_release nogil;

    for (auto &t : threads_)
        t.join();
}

void script_butler::async_search(vector<py::module> &sources)
{
    for (const auto &m : sources) {
        threads_.emplace_back([&m, wanted = wanted_, bw_instance = this]() {
            /* Required whenever we need to run anything Python. */
            py::gil_scoped_acquire gil;

            try {
                m.attr("find")(wanted, bw_instance);
            } catch (const py::error_already_set &err) {
                bw_instance->logger_->error("module '{}' did something wrong:\n{}\n; ignoring...",
                    m.attr("__name__").cast<string>(), err.what());
            }
        });
    }
}

void script_butler::add_item(std::tuple<bookwyrm::nonexacts_t, bookwyrm::exacts_t> item_comps)
{
    bookwyrm::item item(item_comps);
    if (!item.matches(wanted_))
        return;

    std::lock_guard<std::mutex> guard(items_mutex_);

    items_.push_back(item);
    screen_butler_->update_screens();
}

/* ns butler */
}
