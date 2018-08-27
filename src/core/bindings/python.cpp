#include <fmt/format.h>

#include "../../string.hpp"
#include "../item.hpp"
#include "../plugin_handler.hpp"
#include "../python.hpp"

using namespace bookwyrm;
using std::string;
using std::vector;

namespace detail {

    struct log_wrapper {
    public:
        explicit log_wrapper(core::plugin_handler *instance) : ph_(instance) { assert(ph_ != nullptr); }

        void debug(const std::string &msg) { ph_->log(core::log_level::debug, msg); }
        void warn(const std::string &msg) { ph_->log(core::log_level::warn, msg); }
        void error(const std::string &msg) { ph_->log(core::log_level::err, msg); }

    private:
        core::plugin_handler *ph_;
    };

} // namespace detail

static py::object getattr(const core::item &item, const std::string &key)
{
    /*
     * Exact attributes.
     * No reason to query a wanted item size.
     */
    if (key == "yearmod")
        return py::cast(item.exacts.ymod);
    else if (key == "year")
        return py::cast(item.exacts.year);
    else if (key == "volume")
        return py::cast(item.exacts.volume);
    else if (key == "number")
        return py::cast(item.exacts.number);
    else if (key == "pages")
        return py::cast(item.exacts.pages);

    /* Nonexact attributes */
    else if (key == "authors")
        return py::cast(item.exacts.volume);
    else if (key == "title")
        return py::cast(item.nonexacts.title);
    else if (key == "series")
        return py::cast(item.nonexacts.series);
    else if (key == "publisher")
        return py::cast(item.nonexacts.publisher);
    else if (key == "journal")
        return py::cast(item.nonexacts.journal);
    else if (key == "edition")
        return py::cast(item.nonexacts.edition);

    /* TODO: throw something that turns into an actual AttributeError in Python
     * instead. */
    throw std::invalid_argument(std::string("AttributeError: no item attribute with key '") + key + "'");
}

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

    py::class_<core::item>(m, "item").def("__getattr__", &getattr);

    /* core::plugin_handler bindings */

    py::class_<detail::log_wrapper>(m, "log")
        .def("debug", &detail::log_wrapper::debug)
        .def("warn", &detail::log_wrapper::warn)
        .def("error", &detail::log_wrapper::error);

    py::class_<core::plugin_handler>(m, "bookwyrm")
        .def("feed", &core::plugin_handler::add_item)
        .def("__getattr__", [&](core::plugin_handler &ph, const std::string &key) {
            /* TODO: Don't create a new wrapper instance every time */
            if (key == "log") {
                return detail::log_wrapper(&ph);
            }

            throw std::invalid_argument(std::string("AttributeError: no item attribute with key '") + key + "'");
        });
}
