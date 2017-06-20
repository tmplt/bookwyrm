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

class option;
class option_group;
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
    using cli_type = std::unique_ptr<parser>;
    static cli_type make(const string &&scriptname, const groups &&groups);

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

    /* Check if an option has been passed. */
    inline bool has(const string &option) const
    {
        return passed_opts_.find(option) != passed_opts_.cend();
    }

    bool has(size_t index) const
    {
        return positional_args_.size() > index;
    }


    /* Get the value for a given option. */
    string get(string opt) const;
    inline string get(size_t index) const
    {
        return has(index) ? positional_args_[index] : "";
    }

    /* Get all values for a given option. (i.e. --author) */
    vector<string> get_many(const string &&opt) const;

private:

    /*
     * Is the flag valid? If so, is it given in its short
     * or long form?
     */
    static inline bool is(const string_view &option, string opt_short, string opt_long)
    {
        return is_short(option, std::move(opt_short)) || is_long(option, std::move(opt_long));
    }

    static inline bool is_short(const string_view &option, const string_view &opt_short)
    {
        return option.compare(0, opt_short.length(), opt_short) == 0;
    }

    static inline bool is_long(const string_view &option, const string_view &opt_long)
    {
        return option.compare(0, opt_long.length(), opt_long) == 0;
    }

    /* Is the given option value a valid one? If so, return it. Otherwise throw a value_error. */
    static auto check_value(const string_view &flag, const string_view &value, const vector<string> &values);

    /*
     * Parse a single argument with the next argument, which may be its value (or another flag).
     * and emplace the valid ones into passed_opts_.
     */
    void parse(const string_view &input, const string_view &input_next);

    /* Program synopsis. */
    const string synopsis_;

    const options valid_opts_;
    const groups valid_groups_;
    std::multimap<string, string> passed_opts_;
    vector<string> positional_args_;

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
