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

#include <iostream>
#include <algorithm>
#include <iomanip>
#include <utility>

#include "components/command_line.hpp"
#include "fmt/format.h"

/* Create the instance */
cliparser::make_type cliparser::make(string &&progname, const options &&opts)
{
    return std::make_unique<cliparser>(
        "Usage: " + progname + " OPTION...", std::forward<decltype(opts)>(opts)
    );
}

/* Construct the parser. */
cliparser::parser(string &&synopsis, const options &&opts)
    : synopsis_(std::forward<decltype(synopsis)>(synopsis)), valid_opts_(std::forward<decltype(opts)>(opts))
{
}

/* Print program usage message. */
void cliparser::usage() const
{
    std::cout << synopsis_ << "\n\n";

    size_t maxlen = 0;

    /*
     * Get the length of the longest string in the flag column
     * which is used to align the description fields.
     */
    for (const auto &opt : valid_opts_) {
        size_t len = opt.flag_long.length() + opt.flag.length() + 4;
        maxlen = std::max(len, maxlen);
    }

    /*
     * Print each option, its description and token and possible
     * values for said token (if any).
     */
    for (const auto &opt : valid_opts_) {
        /* Padding between flags and description. */
        size_t pad = maxlen - opt.flag_long.length() - opt.token.length();

        fmt::print("  {}, {}", opt.flag, opt.flag_long);

        if (!opt.token.empty()) {
            std::cout << '=' << opt.token;
            pad--;
        }

        /*
         * Output the list with accepted values.
         * This line is printed below the description.
         */
        if (!opt.values.empty()) {
            std::cout << std::setw(pad + opt.desc.length())
                      << opt.desc << '\n';

            pad = pad + opt.flag_long.length() + opt.token.length() + 7;

            std::cout << string(pad, ' ') << opt.token << " is one of: ";
            for (auto &v : opt.values)
                std::cout << v << (v != opt.values.back() ? ", " : "");
        } else {
            std::cout << std::setw(pad + opt.desc.length()) << opt.desc;
        }

        std::cout << std::endl;
    }
}


/* Check if the passed option was provided. */
bool cliparser::has(const string &option) const
{
    return passed_opts_.find(option) != passed_opts_.end();
}

/* Gets the value for a given option. */
string cliparser::get(string opt) const
{
    if (has(std::forward<string>(opt)))
        return passed_opts_.find(opt)->second;

    return "";
}

/* Compare option value with given string. */
bool cliparser::compare(string opt, const string_view &val) const
{
    return get(std::move(opt)) == val;
}

/* Compare option with its short version. */
auto cliparser::is_short(const string_view &option, const string_view &opt_short) const
{
    return option.compare(0, opt_short.length(), opt_short) == 0;
}

/* Compare option with its long version */
auto cliparser::is_long(const string_view &option, const string_view &opt_long) const
{
    return option.compare(0, opt_long.length(), opt_long) == 0;
}

/* Compare with both versions. */
auto cliparser::is(const string_view &option, string opt_short, string opt_long) const
{
    return is_short(option, std::move(opt_short)) || is_long(option, std::move(opt_long));
}

/* Process argument vector. */
void cliparser::process_arguments(const vector<string> &args)
{
    for (size_t i = 0; i < args.size(); i++) {
        const string_view &arg = args[i];
        const string_view &next_arg = args.size() > i + 1 ? args[i + 1] : "";

        parse(arg, next_arg);
    }
}

/* Parse option value. */
auto cliparser::parse_value(string input, const string_view &input_next, choices values) const
{
    string opt = std::move(input);
    size_t pos;
    string_view value;

    if (input_next.empty() && opt.compare(0, 2, "--") != 0)
        throw value_error("Missing value for " + string(opt.data()));
    else if ((pos = opt.find('=')) == string_view::npos && opt.compare(0, 2, "--") == 0)
        throw value_error("Missing value for " + string(opt.data()));
    else if (pos == string::npos && !input_next.empty())
        value = input_next;
    else {
        value = opt.substr(pos + 1);
        opt = opt.substr(0, pos);
    }

    if (!values.empty() && std::find(values.begin(), values.end(), value) == values.end())
        throw value_error(
            "Invalid value '" + string(value.data()) + "' for argument " + string(opt.data())
        );

    return value;
}

/* Parse and validate passed arguments and flags. */
void cliparser::parse(const string_view &input, const string_view &input_next)
{
    if (skipnext_) {
        skipnext_ = false;

        if (!input_next.empty())
            return;
    }

    for (auto &&opt : valid_opts_) {
        if (is(input, opt.flag, opt.flag_long)) {
            if (opt.token.empty()) {
                passed_opts_.insert(std::make_pair(opt.flag_long.substr(2), ""));
            } else {
                auto value = parse_value(input.data(), input_next, opt.values);
                skipnext_ = (value == input_next);
                passed_opts_.insert(make_pair(opt.flag_long.substr(2), value));
            }

            return;
        }
    }

    if (input.compare(0, 1, "-") == 0)
        throw argument_error("Unrecognized option " + string(input.data()));
}
