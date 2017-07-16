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

#include <array>
#include <experimental/filesystem>

#include "utils.hpp"
#include "python.hpp"
#include "components/script_butler.hpp"

namespace fs = std::experimental::filesystem;

namespace bookwyrm {

script_butler::script_butler(const item &wanted)
    : wanted_(wanted) {}

vector<pybind11::module> script_butler::load_sources()
{
#ifdef DEBUG
    /* Bookwyrm must be run from build/ in DEBUG mode. */
    const auto source_path = fs::canonical(fs::path("../src/sources"));
#else
    const std::array<fs::path, 2> paths = {"/etc/bookwyrm/sources",
                                           "~/.config/bookwyrm/sources"};

    // TODO: follow <https://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html>
#endif

    /*
     * Append source_path to Python's sys.path,
     * allowing them to be imported.
     */
    auto sys_path = py::reinterpret_borrow<py::list>(py::module::import("sys").attr("path"));
    sys_path.append(source_path.c_str());

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
    for (const fs::path &p : fs::directory_iterator(source_path)) {
        if (p.extension() != ".py") continue;

        if (!utils::readable_file(p)) {
            logger_->warn("can't load module '{}': not a regular file or unreadable"
                    "; ignoring...", p.string());
            continue;
        }

        try {
            string module = p.stem();
            logger_->debug("loading module '{}'...", module);
            sources.emplace_back(py::module::import(module.c_str()));
        } catch (const py::error_already_set &err) {
            logger_->warn("{}; ignoring...", err.what());
        }
    }

    if (sources.empty())
        throw program_error("couldn't find any valid source modules, terminating...");

    return sources;
}

script_butler::~script_butler()
{
    /*
     * Current behaviour is that the program cannot terminate unless
     * all source threads has joined. We'll of course want do to this.
     *
     * It's not possible to end a std::thread in a smooth matter. We could
     * instead have a control variable that the source scripts check every
     * once in a while. When this is set upon quitting the curses UI, we'll
     * have to let each script handle its own termination.
     *
     * Not joining the threads here doesn't make the OS very happy.
     */
    py::gil_scoped_release nogil;

    for (auto &t : threads_)
        t.join();
}

void script_butler::async_search(vector<py::module> &sources)
{
    for (const auto &m : sources) {
        try {
            threads_.emplace_back([&m, wanted = wanted_, bw_instance = this]() {
                /* Required whenever we need to run anything Python. */
                py::gil_scoped_acquire gil;
                m.attr("find")(wanted, bw_instance);
            });
        } catch (const py::error_already_set &err) {
            logger_->error("module '{}' did something wrong ({}); ignoring...",
                    m.attr("__name__").cast<string>(), err.what());
            continue;
        }
    }
}

void script_butler::add_item(std::tuple<nonexacts_t, exacts_t> item_comps)
{
    item item(item_comps);

    std::lock_guard<std::mutex> guard(items_mutex_);
    /* if (!item.matches(wanted_)) */
    /*     return; */

    items_.push_back(item);
    screen_butler_->update_screens();
}

/* ns bookwyrm */
}
