/*
 * A convenience header when we want to define
 * errors with a more specific name.
 *
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
