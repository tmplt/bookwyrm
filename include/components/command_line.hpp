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

#pragma once

#include <map>                       // std::map
#include <string>                    // std::string
#include <vector>                    // std::vector
#include <memory>                    // std:unique_ptr
#include <utility>                   // std::forward
#include <experimental/string_view>  // std::experimental::string_view

#include "errors.hpp"

using std::string;
using std::vector;
using std::experimental::string_view;

using std::forward;

namespace command_line {

/* Something more specific than "runtime_error". */
DEFINE_ERROR(argument_error);
DEFINE_ERROR(value_error);

class option;
class option_group;
using options = vector<option>;
using groups  = vector<option_group>;
using choices = vector<string>;

/*
 * Holds all properties for an option.
 * (A possible flag to pass to the program.)
 */
struct option {
    const string flag, flag_long, desc;

    /*
     * e.g. seen as --log LEVEL in usage(),
     * where LEVEL is the token.
     */
    const string token;

    /*
     * For options where a set of values are allowed,
     * e.g. --log LEVEL, where LEVEL is one of:
     * error, warning, info, trace.
     */
    const choices values;

    explicit option(string &&flag, string &&flag_long, string &&desc, string &&token = "",
            const choices c = {})
        : flag(forward<string>(flag)), flag_long(forward<string>(flag_long)),
        desc(forward<string>(desc)), token(forward<string>(token)), values(c) {}
};

/* A named group with it's related options. */
struct option_group {
public:
    const string name;
    const string synopsis;
    vector<option> options;

    explicit option_group(string &&name, string &&synopsis)
        : name(forward<string>(name)), synopsis(forward<string>(synopsis)) {}

    template <typename... Args>
    option_group& operator()(Args&&... args)
    {
        options.emplace_back(option(forward<Args>(args)...));

        return *this;
    }
};

class parser {
public:
    using cli_type = std::unique_ptr<parser>;
    static cli_type make(const string &&scriptname, const groups &&groups);

    /* Construct the parser. */
    explicit parser(string &&synopsis, const groups &&groups)
        : synopsis_(std::forward<decltype(synopsis)>(synopsis)),
        valid_groups_(std::forward<decltype(groups)>(groups)) {}

    /* Print which flags you can pass and how to use the program. */
    void usage() const;

    /* Process command line arguments. */
    void process_arguments(const vector<string> &args);

    /*
     * Program specific; adhere to the options' synopsises seen in main(),
     * e.g. pass at least one main argument if -d isn't passed.
     */
    void validate_arguments() const;

    /* Check if an option has been passed. */
    bool has(const string &option) const;

    /* Get the value for a given option. */
    string get(string opt) const;

protected:
    /*
     * Construct a string of all valid token values,
     * e.g. "VAL1, VAL2, VAL3".
     */
    auto values_to_str(const choices &values) const;

    /*
     * Is the flag passed in it's long or its short form?
     * Does it match any of the two?
     */
    auto is_short(const string_view &option, const string_view &opt_short) const;
    auto is_long(const string_view &option, const string_view &opt_long) const;
    auto is(const string_view &option, string opt_short, string opt_long) const;

    /* Check if the passed value matches an element in the group of valid values. */
    auto check_value(const string_view &flag, const string_view &value, const choices &values) const;

    /* Parse a single argument with the next argument, which may be its value (or another flag). */
    void parse(const string_view &input, const string_view &input_next);

    /*
     * Used to check if a passed option is valid
     * between the group of valid options and the group
     * of passed options.
     */
    bool compare(string opt, const string_view &val) const;

private:
    /* Program synopsis. */
    const string synopsis_;

    const options valid_opts_;
    const groups valid_groups_;
    std::map<string, string> passed_opts_;

    /*
     * Is the next argument associated with the previous one,
     * or should the current argument be treated as another flag?
     */
    bool skipnext_ = false;
};

/* ns command_line */
}

using cliparser = command_line::parser;
using cligroup  = command_line::option_group;
using clioption = command_line::option;
