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

#include <iostream>   // std::cout, std::endl
#include <algorithm>  // std::max(), std::set_intersection()
#include <iomanip>    // std::setw()
#include <utility>    // std::make_unique<T>(), std::forward<T>()

#include "components/command_line.hpp"
#include "fmt/format.h"

/* Create the instance. */
cliparser::cli_type cliparser::make(string &&progname, const groups &&groups)
{
    return std::make_unique<cliparser>(
        "Usage: " + progname + " OPTION...", std::forward<decltype(groups)>(groups)
    );
}

/*
 * Return a string in the form of "VAL1, VAL2, VAL3".
 */
auto cliparser::values_to_str(const choices &values) const
{
    string retstring = "";
    for (const auto &v : values)
        retstring += v + (v != values.back() ? ", " : "");

    return retstring;
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
    for (const auto &group : valid_groups_) {
        for (const auto &opt : group.options) {
            size_t len = opt.flag_long.length() + opt.flag.length() +
                         opt.token.length() + 4;

            maxlen = std::max(len, maxlen);
        }
    }

    /*
     * Print each option group, its options, the options'
     * descriptions, token and possible values for said token (if any).
     */
    for (const auto &group : valid_groups_) {
        fmt::print("{} arguments - {}:\n", group.name, group.synopsis);

        for (const auto &opt : group.options) {
            /* Padding between flags and description. */
            size_t pad = maxlen - opt.flag_long.length() - opt.token.length();

            fmt::print("  {}, {}", opt.flag, opt.flag_long);

            if (!opt.token.empty()) {
                std::cout << ' ' << opt.token;
                pad--;
            }

            /*
             * Print the list with accepted values.
             * This line is printed below the description.
             */
            if (!opt.values.empty()) {
                std::cout << std::setw(pad + opt.desc.length())
                          << opt.desc << '\n';

                pad += opt.flag_long.length() + opt.token.length() + 7;

                std::cout << string(pad, ' ') << opt.token << " is one of: "
                          << values_to_str(opt.values);
            } else {
                std::cout << std::setw(pad + opt.desc.length()) << opt.desc;
            }

            std::cout << std::endl;
        }

        std::cout << '\n';
    }
}

/* Check if an option has been passed. */
bool cliparser::has(const string &option) const
{
    return passed_opts_.find(option) != passed_opts_.end();
}

/* Gets the value for the given option. */
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

/*
 * Compare passed option with it's short and long flag.
 * Returns true if option is valid.
 */
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

/* Check whether the the option groups' synopsises are adhered to. */
void cliparser::validate_arguments() const
{
    vector<string> passed_opts;
    vector<string> required_opts;
    vector<string> intersection;

    for (const auto &opt : passed_opts_) {
        /* We don't want the value. */
        passed_opts.emplace_back(opt.first);
    }

    /* 0 = the "Main" group. Enum it? */
    for (const auto &opt : valid_groups_[0].options) {
        required_opts.emplace_back(opt.flag_long.substr(2));
    }

    std::sort(passed_opts.begin(), passed_opts.end());
    std::sort(required_opts.begin(), required_opts.end());

    /* Which of the required options have been passed, if any? */
    std::set_intersection(passed_opts.begin(), passed_opts.end(),
            required_opts.begin(), required_opts.end(),
            std::back_inserter(intersection));

    bool ident_passed = std::find(passed_opts.begin(), passed_opts.end(), "ident") !=
        passed_opts.end();

    if (ident_passed && passed_opts.size() > 1)
        throw argument_error("ident flag is exclusive and may not be passed with another flag");

    if (intersection.empty() && !passed_opts.empty())
        throw argument_error("at least one main argument must be specified");
}

/* Get the value of the option. */
auto cliparser::get_value(const string_view &flag, const string_view &value, const choices &values) const
{

    if (value.empty())
        throw value_error("missing value for " + string(flag.data()));

    if (!values.empty() && std::find(values.begin(), values.end(), value) == values.end()) {
        throw value_error(
            "invalid value '" + string(value.data()) + "' for argument " + string(flag.data()) +
            "; valid options are: " + values_to_str(values)
        );
    }

    return value;
}

/* Parse and validate given option. */
void cliparser::parse(const string_view &input, const string_view &input_next)
{
    if (skipnext_) {
        skipnext_ = false;
        return;
    }

    for (const auto &group : valid_groups_) {
        for (const auto &opt : group.options) {
            if (is(input, opt.flag, opt.flag_long)) {
                if (opt.token.empty()) {
                    /* The option is only a flag. */
                    passed_opts_.insert(std::make_pair(opt.flag_long.substr(2), ""));
                } else {
                    /* The option should have an accompanied value. */
                    auto value = get_value(input, input_next, opt.values);
                    skipnext_ = (value == input_next);
                    passed_opts_.insert(make_pair(opt.flag_long.substr(2), value));
                }

                return;
            }
        }
    }

    throw argument_error("unrecognized option " + string(input.data()));
}
