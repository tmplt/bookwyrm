#include <fmt/format.h>

#include "../../string.hpp"
#include "../item.hpp"
#include "../plugin_handler.hpp"
#include "../python.hpp"

using namespace bookwyrm;
using std::string;
using std::vector;

namespace detail {

    struct __attribute__((visibility("hidden"))) log_wrapper {
    public:
        explicit log_wrapper(core::plugin_handler *instance) : ph_(instance) { assert(ph_ != nullptr); }

        void debug(const std::string &msg) { ph_->log(core::log_level::debug, msg); }
        void warn(const std::string &msg) { ph_->log(core::log_level::warn, msg); }
        void error(const std::string &msg) { ph_->log(core::log_level::err, msg); }

    private:
        core::plugin_handler *ph_;
    };

    py::dict to_py_dict(const core::item &item)
    {
        py::dict dict;

        /* Exacts attributes */
        dict["year_mod"] = py::cast(item.exacts.ymod);
        std::array<std::pair<string, int>, 4> exact_pairs = {{{"year", item.exacts.year},
                                                              {"volume", item.exacts.volume},
                                                              {"number", item.exacts.number},
                                                              {"pages", item.exacts.pages}}};
        for (auto &pair : exact_pairs) {
            if (std::get<1>(pair) != core::empty)
                dict[std::get<0>(pair).c_str()] = std::get<1>(pair);
        }
        if (!item.exacts.extension.empty())
            dict["extension"] = py::cast(item.exacts.extension);

        /* /1* Nonexact attributes *1/ */
        if (!item.nonexacts.authors.empty())
            dict["authors"] = py::cast(item.nonexacts.authors);
        std::array<std::pair<string, string>, 5> nonexact_pairs = {{{"title", item.nonexacts.title},
                                                                    {"series", item.nonexacts.series},
                                                                    {"publisher", item.nonexacts.publisher},
                                                                    {"journal", item.nonexacts.journal},
                                                                    {"edition", item.nonexacts.edition}}};
        for (auto &pair : nonexact_pairs) {
            if (!std::get<1>(pair).empty())
                dict[std::get<0>(pair).c_str()] = py::cast(std::get<1>(pair).c_str());
        }

        return dict;
    }

} // namespace detail

PYBIND11_MODULE(pybookwyrm, m)
{
    m.attr("__doc__") = "bookwyrm python bindings";

    /* core::item bindings */

    m.attr("empty") = core::empty;

    py::enum_<core::year_mod>(m, "yearmod")
        .value("unused", core::year_mod::unused)
        .value("equal", core::year_mod::equal)
        .value("eq_gt", core::year_mod::eq_gt)
        .value("eq_lt", core::year_mod::eq_lt)
        .value("lt", core::year_mod::lt)
        .value("gt", core::year_mod::gt);

    /* core::plugin_handler bindings */

    py::class_<detail::log_wrapper>(m, "log")
        .def("debug", &detail::log_wrapper::debug)
        .def("warn", &detail::log_wrapper::warn)
        .def("error", &detail::log_wrapper::error);

    py::class_<core::plugin_handler>(m, "bookwyrm")
        .def("feed", [&m](core::plugin_handler &ph, py::dict dict) {
            /* Find the base name of the plugin calling this function. */
            auto sys = py::module::import("sys");
            auto os = py::module::import("os");
            py::str filepath = sys.attr("_getframe")(0).attr("f_code").attr("co_filename");
            py::str basename = os.attr("path").attr("basename")(filepath);

            dict["origin_plugin"] = basename;
            ph.add_item(std::move(dict));
        })
        .def("__getattr__", [&](core::plugin_handler &ph, const std::string &key) {
            /* TODO: Don't create a new wrapper instance every time */
            if (key == "log") {
                return detail::log_wrapper(&ph);
            }

            throw std::invalid_argument(std::string("AttributeError: no item attribute with key '") + key + "'");
        });
}
