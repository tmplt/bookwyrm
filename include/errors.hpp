/*
 * A convenience header when we want to define
 * errors with a more specific name.
 */

#pragma once

#include <stdexcept>
#include <string>

#define DEFINE_ERROR(error)                                      \
    class error : public std::runtime_error {                    \
    public:                                                      \
        explicit error(const std::string &message, int code = 0) \
            : runtime_error(message), code(code) {}              \
        virtual ~error() {}                                      \
        int code{0};                                             \
    }

#define DEFINE_CHILD_ERROR(error, parent) \
    class error : public parent {         \
        using parent::parent;             \
    }

DEFINE_ERROR(program_error);
DEFINE_ERROR(argument_error);

DEFINE_CHILD_ERROR(value_error, argument_error);
DEFINE_CHILD_ERROR(component_error, program_error);
