#pragma once

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace bookwyrm::core {
    struct item;
}

namespace detail {
    py::dict to_py_dict(const bookwyrm::core::item &item);
}
