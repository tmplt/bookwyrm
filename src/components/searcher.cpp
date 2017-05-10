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

#include <iostream>
#include <system_error>
#include <cerrno>
#include <experimental/filesystem>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>
#include <array>

#include "components/searcher.hpp"
#include "utils.hpp"

namespace fs = std::experimental::filesystem;
namespace py = pybind11;

namespace bookwyrm {

searcher::searcher(const item &wanted)
    : wanted_(wanted)
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
            /* It's not a Python module. */
            continue;
        }

        if (!utils::valid_file(p)) {
            _logger->warn("can't load module '{}': not a regular file or unreadable"
                    "; ignoring...",
                    module_file);
            continue;
        }

        module_file.resize(file_ext_pos);

        try {
            _logger->debug("loading module '{}'...", module_file);
            sources_.emplace_back(py::module::import(module_file.c_str()));
        } catch (const py::error_already_set &err) {
            _logger->warn("{}; ignoring...", err.what());
        }
    }

    if (sources_.empty())
        throw program_error("couldn't find any sources, terminating...");
}

void searcher::test_sources()
{
    for (const auto &m : sources_) {
        try {
            auto item_comps = m.attr("find")(wanted_).cast<std::tuple<nonexacts_t, exacts_t>>();
            items_.emplace_back(item_comps);
        } catch (const py::cast_error &err) {
            _logger->error("tuple cast from module '{}' failed: {}; ignoring...",
                    m.attr("__name__").cast<string>(), err.what());
            continue;
        } catch (const py::error_already_set &err) {
            _logger->error("module '{}' didn't return a tuple ({}); ignoring...",
                    m.attr("__name__").cast<string>(), err.what());
            continue;
        }
    }
}

/* ns bookwyrm */
}
