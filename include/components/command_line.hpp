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

DEFINE_ERROR(argument_error);
DEFINE_ERROR(value_error);

class option;
class option_group;
using options = vector<option>;
using groups  = vector<option_group>;
using choices = vector<string>;
using values  = std::map<string, string>;

class option {
public:
    string flag;
    string flag_long;
    string desc;

    /*
     * e.g. seen as --log LEVEL in usage(),
     * where LEVEL is the token.
     */
    string token;

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

class option_group {
public:
    string name;
    string synopsis;
    vector<option> options;

    explicit option_group(string &&name, string &&synopsis, vector<option> options)
        : name(forward<string>(name)), synopsis(forward<string>(synopsis)),
                options(forward<vector<option>>(options)) {}

    template <typename... Args>
    void add_arg(Args&&... args)
    {
        auto opt = option(std::forward<Args>(args)...);
        options.emplace_back(std::move(opt));
    }
};

class parser {
public:
    using cli_type = std::unique_ptr<parser>;
    static cli_type make(string &&scriptname, const groups &&groups);

    /* Construct the parser. */
    explicit parser(string &&synopsis, const groups &&groups)
        : synopsis_(std::forward<decltype(synopsis)>(synopsis)),
        valid_groups_(std::forward<decltype(groups)>(groups)) {}

    void usage() const;

    void process_arguments(const vector<string> &args);

    bool has(const string &option) const;
    string get(string opt) const;
    bool compare(string opt, const string_view &val) const;

protected:
    auto values_to_str(const choices &values) const;

    auto is_short(const string_view &option, const string_view &opt_short) const;
    auto is_long(const string_view &option, const string_view &opt_long) const;
    auto is(const string_view &option, string opt_short, string opt_long) const;

    auto get_value(const string_view &flag, const string_view &value, const choices &values) const;
    void parse(const string_view &input, const string_view &input_next);

private:
    string synopsis_;
    const options valid_opts_;
    const groups valid_groups_;
    values passed_opts_;
    bool skipnext_ = false;
};

/* ns command_line */
}

using cliparser = command_line::parser;
using cligroup  = command_line::option_group;
using clioption = command_line::option;
