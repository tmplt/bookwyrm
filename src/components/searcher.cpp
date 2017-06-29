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
#include "components/searcher.hpp"

namespace fs = std::experimental::filesystem;

namespace bookwyrm {

searcher::searcher(const item &wanted)
    : wanted_(wanted) {}

void searcher::load_sources()
{
#ifdef DEBUG
    /* Bookwyrm must be run from build/. */
    const auto source_path = fs::canonical(fs::path("../src/sources"));
#else
    const std::array<fs::path, 2> paths = {"/etc/bookwyrm/sources",
                                           "~/.config/bookwyrm/sources"};
#endif
    /* Append source_path to Python's sys.path. */
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
    string module_file;
    for (const fs::path &p : fs::directory_iterator(source_path)) {
        module_file = p.filename();
        auto file_ext_pos = module_file.rfind(".py");

        if (file_ext_pos == string::npos) {
            /*
             * It's not a Python module.
             * (or at least doesn't contain ".py"
             */
            continue;
        }

        if (!utils::readable_file(p)) {
            logger_->warn("can't load module '{}': not a regular file or unreadable"
                    "; ignoring...",
                    module_file);
            continue;
        }

        module_file.resize(file_ext_pos);

        try {
            logger_->debug("loading module '{}'...", module_file);
            sources_.emplace_back(py::module::import(module_file.c_str()));
        } catch (const py::error_already_set &err) {
            logger_->warn("{}; ignoring...", err.what());
        }
    }

    if (sources_.empty())
        throw program_error("couldn't find any sources, terminating...");
}

searcher::~searcher()
{
    /*
     * Current behaviour is that the program cannot terminate unless
     * all source threads has joined. We'll of course want do to this.
     *
     * It's not possible to end a std::thread in a smooth matter. We could
     * instead have a control variable that the source scripts check every
     * once in a while. When this is set upon quitting the curses UI, we'll
     * have to let each script handle its own termination.
     */
    py::gil_scoped_release nogil;

    for (auto &t : threads_)
        t.join();
}

void searcher::async_search()
{
    for (const auto &m : sources_) {
        try {
            threads_.emplace_back([m, this]() {
                py::gil_scoped_acquire gil;
                m.attr("find")(this->wanted_, this);
            });
        } catch (const py::error_already_set &err) {
            logger_->error("module '{}' did something wrong ({}); ignoring...",
                    m.attr("__name__").cast<string>(), err.what());
            continue;
        }
    }
}

void searcher::add_item(std::tuple<nonexacts_t, exacts_t> item_comps)
{
    item item(item_comps);

    std::lock_guard<std::mutex> guard(items_mutex_);
    /* if (!item.matches(wanted_)) */
    /*     return; */

    items_.push_back(item);
    menu_->update();
}

/* ns bookwyrm */
}
