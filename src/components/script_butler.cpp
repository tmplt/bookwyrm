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

script_butler::script_butler(const bookwyrm::item &&wanted, logger_t logger)
    : logger_(logger), wanted_(wanted) {}

vector<pybind11::module> script_butler::load_seekers()
{
    vector<fs::path> seeker_paths;
#ifdef DEBUG
    /* Bookwyrm must be run from build/ in DEBUG mode. */
    seeker_paths = { fs::canonical(fs::path("../src/seekers")) };
#else
    /* TODO: look through /etc/bookwyrm/seekers/ also. */
    if (fs::path conf = std::getenv("XDG_CONFIG_HOME"); !conf.empty())
        seeker_paths.push_back(conf / "bookwyrm/seekers");
    else if (fs::path home = std::getenv("HOME"); !home.empty())
        seeker_paths.push_back(home / ".config/bookwyrm/seekers");
    else
        logger_->error("couldn't find any seeker script directories.");
#endif

    /*
     * Append the seeker paths to Python's sys.path,
     * allowing them to be imported.
     */
    auto sys_path = py::reinterpret_borrow<py::list>(py::module::import("sys").attr("path"));
    for (auto &p : seeker_paths)
        sys_path.append(p.string().c_str());

    /*
     * Find all Python modules and populate the
     * list of seekers by loading them.
     *
     * The first occurance of a module will be imported,
     * the latter ones will be ignored by Python. So we
     * either need to prepend the paths to sys.path, or
     * make sure that we don't clash with the many module
     * names in Python.
     */
    vector<py::module> seekers;
    for (const auto &seeker_path : seeker_paths) {
        for (const fs::path &p : fs::directory_iterator(seeker_path)) {
            if (p.extension() != ".py") continue;

            if (!utils::readable_file(p)) {
                logger_->error("can't load module '{}': not a regular file or unreadable"
                        "; ignoring...", p.string());
                continue;
            }

            try {
                string module = p.stem();
                logger_->debug("loading module '{}'...", module);
                seekers.emplace_back(py::module::import(module.c_str()));
            } catch (const py::error_already_set &err) {
                logger_->error("{}; ignoring...", err.what());
            }
        }
    }

    if (seekers.empty())
        throw program_error("couldn't find any valid seeker scripts");

    return seekers;
}

script_butler::~script_butler()
{
    py::gil_scoped_release nogil;

    for (auto &t : threads_)
        t.detach();
}

void script_butler::async_search(vector<py::module> &seekers)
{
    for (const auto &m : seekers) {
        threads_.emplace_back([&m, wanted = wanted_, bw_instance = this]() {
            /* Required whenever we need to run anything Python. */
            py::gil_scoped_acquire gil;

            try {
                m.attr("find")(wanted, bw_instance);
            } catch (const py::error_already_set &err) {
                if (!err.matches(PyExc_GeneratorExit)) {
                    /* Module terminated abnormally. */
                    bw_instance->logger_->error("module '{}' did something wrong: {}; ignoring...",
                        m.attr("__name__").cast<string>(), err.what());
                }
            }
        });
    }
}

void script_butler::add_item(std::tuple<bookwyrm::nonexacts_t, bookwyrm::exacts_t, bookwyrm::misc_t> item_comps)
{
    bookwyrm::item item(item_comps);
    if (!item.matches(wanted_) || item.misc.uris.size() == 0)
        return;

    std::lock_guard<std::mutex> guard(items_mutex_);

    items_.push_back(item);

    if (!screen_butler_.expired()) {
        const auto screen = screen_butler_.lock();
        screen->repaint_screens();
    }
}

void script_butler::log_entry(spdlog::level::level_enum lvl, string msg)
{
    logger_->log(lvl, msg);
}

/* ns butler */
}
