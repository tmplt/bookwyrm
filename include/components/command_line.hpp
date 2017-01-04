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

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <experimental/string_view>

#include "errors.hpp"

using std::string;
using std::vector;
using std::experimental::string_view;

namespace command_line {

DEFINE_ERROR(argument_error);
DEFINE_ERROR(value_error);

class option;
using options = vector<option>;
using choices = vector<string>;

using values  = std::map<string, string>;

class option {
public:
    string flag;
    string flag_long;
    string desc;

    /*
     * e.g. seen as --log=LEVEL in usage(),
     * where LEVEL is the token.
     */
    string token;

    /*
     * For options where a set of values are allowed,
     * e.g. --log=LEVEL, where LEVEL is one of:
     * error, warning, info, trace.
     */
    const choices values;

    explicit option(string flag, string flag_long, string desc, string token = "", const choices c = {})
        : flag(flag), flag_long(flag_long), desc(desc), token(token), values(c) {}
};

class parser {
public:
    using make_type = std::unique_ptr<parser>;
    static make_type make(string &&scriptname, const options &&opts);

    explicit parser(string &&synposis, const options &&opts);

    void usage() const;

    void process_arguments(const vector<string> &args);

    bool has(const string &option) const;
    string get(string opt) const;
    bool compare(string opt, const string_view &val) const;

protected:
    auto is_short(const string_view &option, const string_view &opt_short) const;
    auto is_long(const string_view &option, const string_view &opt_long) const;
    auto is(const string_view &option, string opt_short, string opt_long) const;

    auto parse_value(string opt, const string_view &input_next, choices values) const;
    void parse(const string_view &input, const string_view &input_next);

private:
    string synopsis_;
    const options valid_opts_;
    values passed_opts_;
    bool skipnext_ = false;
};

/* ns command_line */
}

using cliparser  = command_line::parser;
using clioption  = command_line::option;
using clioptions = command_line::options;
