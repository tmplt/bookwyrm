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

#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "storage.hpp"
#include "item.hpp"
using namespace bookwyrm;

void feed(/*const item wanted*/)
{
    /*
     * Some function like this will be used to
     * get the items from the Python scripts.
     */
}

/* Are these only deprecated in dean0x7d's branch? */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
PYBIND11_PLUGIN(pybookwyrm)
{
    py::module m("pybookwyrm", "data structures used by bookwyrm");

    py::enum_<year_mod>(m, "year_mod")
        .value("equal", year_mod::equal)
        .value("eq_gt", year_mod::eq_gt)
        .value("eq_lt", year_mod::eq_lt)
        .value("lt",    year_mod::lt)
        .value("gt",    year_mod::gt);

    py::class_<exacts_t>(m, "exacts_t")
        .def_readwrite("year",    &exacts_t::year)
        .def_readwrite("edition", &exacts_t::edition)
        .def_readwrite("ext",     &exacts_t::ext)
        .def_readwrite("volume",  &exacts_t::volume)
        .def_readwrite("number",  &exacts_t::number)
        .def_readwrite("pages",   &exacts_t::pages)
        .def_readwrite("lang",    &exacts_t::lang);

    py::class_<nonexacts_t>(m, "nonexacts_t")
        .def_readwrite("authors",   &nonexacts_t::authors)
        .def_readwrite("title",     &nonexacts_t::title)
        .def_readwrite("serie",     &nonexacts_t::serie)
        .def_readwrite("publisher", &nonexacts_t::publisher)
        .def_readwrite("journal",   &nonexacts_t::journal);

    py::class_<item>(m, "item")
        .def_readwrite("nonexacts", &item::nonexacts)
        .def_readwrite("exacts",    &item::exacts)
        .def("__repr__",
            [](const item &i) {
                return "<bookwyrm.item with title '" + i.nonexacts.title + "'>";
            }
        );

    /* m.def("feed", &feed, "Feed passed books to the bookwyrm"); */
    /* m.attr("empty") = empty; */

    return m.ptr();
}
#pragma GCC diagnostic pop
