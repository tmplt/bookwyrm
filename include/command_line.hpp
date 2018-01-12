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
#include <memory>
#include <utility>

#include "common.hpp"

using std::forward;

namespace command_line {

struct option;
struct option_group;
using options = vector<option>;
using groups  = vector<option_group>;

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
    const vector<string> values;

    explicit option(string &&flag, string &&flag_long, string &&desc, string &&token = "",
            vector<string> &&c = {})
        : flag(forward<string>(flag)), flag_long(forward<string>(flag_long)),
        desc(forward<string>(desc)), token(forward<string>(token)),
        values(forward<decltype(c)>(c)) {}
};

/* A named group with it's related options. */
struct option_group {
    const string name;
    const string synopsis;
    vector<option> options;

    explicit option_group(string &&name, string &&synopsis = "")
        : name(forward<string>(name)), synopsis(forward<string>(synopsis)) {}

    template <typename... Args>
    option_group& operator()(Args&&... args)
    {
        /*
         * Since the underlaying type is an option we just have to
         * forward the arguments.
         */
        options.emplace_back(forward<Args>(args)...);
        return *this;
    }
};

class parser {
public:
    /* using cli_type = std::unique_ptr<parser>; */
    static parser make(const string &&scriptname, const groups &&groups);

    // TODO: strip code for valid_groups? (We have no use for it).

    /* Construct the parser. */
    explicit parser(string &&synopsis, const groups &&groups)
        : synopsis_(forward<string>(synopsis)),
        valid_groups_(forward<decltype(groups)>(groups)) {}

    /* Print program usage. */
    void usage() const;

    /* Process command line arguments. */
    void process_arguments(const vector<string> &args);

    /*
     * Program specific; adhere to the options' synopsises seen in main(),
     * e.g. pass at least one main argument if -d isn't passed.
     */
    void validate_arguments() const;

    /* Check if an option/positional argument has been passed. */
    bool has(const string &option) const;
    bool has(size_t index) const;

    /* Get the value for a given option/positional argument. */
    string get(string opt) const;
    string get(size_t index) const;

    /* Get all values for a given option. (i.e. --author) */
    vector<string> get_many(const string &&opt) const;

private:

    /*
     * Parse an argument with the next argument, which may be its value (or another flag).
     * and emplace the valid ones into passed_opts_. Returns true if the next argument
     * from the command line should be ignored.
     */
    bool parse_pair(const string_view &input, const string_view &input_next);

    /* An option must either match its long of short variant to exist. */
    static bool opt_exists(const string_view &option, string opt_short, string opt_long);

    /* Program synopsis. */
    const string synopsis_;

    const options valid_opts_;
    const groups valid_groups_;
    std::multimap<string, string> passed_opts_;
    vector<string> positional_args_;
};

/* ns command_line */
}

using cliparser = command_line::parser;
using cligroup  = command_line::option_group;
using cligroups = command_line::groups;
using valid_opts = std::initializer_list<string>;
