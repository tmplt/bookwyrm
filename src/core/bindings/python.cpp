#include <fmt/format.h>

#include "../python.hpp"
#include "../../string.hpp"
#include "../item.hpp"
#include "../plugin_handler.hpp"

using namespace bookwyrm;
using std::string;
using std::vector;

PYBIND11_MODULE(pybookwyrm, m)
{
    m.attr("__doc__") = "bookwyrm python bindings";

    /* core::item bindings */

    m.attr("empty") = core::empty;

    py::enum_<core::year_mod>(m, "year_mod")
        .value("equal", core::year_mod::equal)
        .value("eq_gt", core::year_mod::eq_gt)
        .value("eq_lt", core::year_mod::eq_lt)
        .value("lt",    core::year_mod::lt)
        .value("gt",    core::year_mod::gt);

    py::class_<core::exacts_t>(m, "exacts_t")
        .def_readonly("year",      &core::exacts_t::year)
        .def_readonly("extension", &core::exacts_t::extension)
        .def_readonly("volume",    &core::exacts_t::volume)
        .def_readonly("number",    &core::exacts_t::number)
        .def_readonly("pages",     &core::exacts_t::pages)
        /* .def_readwrite("lang",    &core::exacts_t::lang) */
        .def("__repr__", [](const core::exacts_t &c) {
            return fmt::format(
                "<pybookwyrm.exacts_t with fields:\n"
                "\tyear:      {}\n"
                "\tfile type: {}\n"
                "\tvolume:    {}\n"
                "\tnumber:    {}\n"
                "\tpages:     {}\n"
                "\tsize:      {}B\n",
                /* "\tlanguage:  {}\n>", */
                c.year, c.extension, c.volume,
                c.number, c.pages, c.size //, c.lang
            );
        });

    py::class_<core::nonexacts_t>(m, "nonexacts_t")
        .def_readonly("authors",   &core::nonexacts_t::authors)
        .def_readonly("title",     &core::nonexacts_t::title)
        .def_readonly("series",    &core::nonexacts_t::series)
        .def_readonly("publisher", &core::nonexacts_t::publisher)
        .def_readonly("journal",   &core::nonexacts_t::journal)
        .def_readonly("edition",   &core::nonexacts_t::edition)
        .def("__repr__", [](const core::nonexacts_t &c) {
            return fmt::format(
                "<pybookwyrm.nonexacts_t with fields:\n"
                "\ttitle:     '{}'\n"
                "\tserie:     '{}'\n"
                "\tedition:   '{}'\n"
                "\tpublisher: '{}'\n"
                "\tjournal:   '{}'\n"
                "\tauthors:   '{}'\n>",
                c.title, c.series, c.edition, c.publisher, c.journal,
                vector_to_string(c.authors)
            );
        });

    py::class_<core::misc_t>(m, "misc_t")
        .def_readonly("isbns", &core::misc_t::isbns)
        .def_readonly("uris", &core::misc_t::uris)
        .def("__repr__", [](const core::misc_t &c) {
            return fmt::format(
                "<pybookwyrn.misc_t with fields:\n"
                "\tisbns:     '{}'\n"
                "\turis:      '{}'\n",
                vector_to_string(c.isbns),
                vector_to_string(c.uris)
            );
        });

    py::class_<core::item>(m, "item")
        .def_readonly("nonexacts", &core::item::nonexacts)
        .def_readonly("exacts",    &core::item::exacts)
        .def("__repr__", [](const core::item &i) {
            return "<bookwyrm.item with title '" + i.nonexacts.title + "'>";
        });

    /* core::plugin_handler bindings */

    py::enum_<core::log_level>(m, "log_level")
        .value("debug", core::log_level::debug)
        .value("info",  core::log_level::info)
        .value("warn",  core::log_level::warn)
        .value("error", core::log_level::err);

    py::class_<core::plugin_handler>(m, "bookwyrm")
        .def("feed",        &core::plugin_handler::add_item)
        .def("log",         &core::plugin_handler::log);
}
