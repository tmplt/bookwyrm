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

#include "utils.hpp"
#include "components/command_line.hpp"

enum {
    /*
     * Same order as they are initialized
     * in main(). Perhaps we should have a
     * getter funtion instead?
     *
     * These (or well, only main) are used in
     * validate_arguments() to get the group of
     * main arguments so that we can find if any of
     * those are passed.
     *
     * I'm a bit unsure if it's worth the time to
     * implement a better way to do this. This will work
     * as long as they are in the same order as they are
     * initialized in in main().
     */
    main,
    excl,
    exact,
    misc
};

enum { /* magic padding numbers */
    padding_margin = 4,
    desc_align_magic = 7
};

cliparser::cli_type cliparser::make(const string &&progname, const groups &&groups)
{
    return std::make_unique<cliparser>(
        "Usage: " + progname + " option... download_path", std::forward<decltype(groups)>(groups)
    );
}

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
                         opt.token.length() + padding_margin;

            maxlen = std::max(len, maxlen);
        }
    }

    /*
     * Print each option group, its options, the options'
     * descriptions, token and possible values for said token (if any).
     */
    for (const auto &group : valid_groups_) {
        std::cout << group.name << " arguments"
                  << (group.synopsis.empty() ? ":" : " - " + group.synopsis + ":")
                  << '\n';

        for (const auto &opt : group.options) {
            /* Padding between flags and description. */
            size_t pad = maxlen - opt.flag_long.length() - opt.token.length();

            std::cout << "  " << opt.flag << ", " << opt.flag_long;

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

                pad += opt.flag_long.length() + opt.token.length() + desc_align_magic;

                std::cout << string(pad, ' ') << opt.token << " is one of: "
                          << utils::vector_to_string(opt.values);
            } else {
                std::cout << std::setw(pad + opt.desc.length()) << opt.desc;
            }

            std::cout << '\n';
        }

        std::cout << std::endl;
    }
}

string cliparser::get(string opt) const
{
    if (has(std::forward<string>(opt)))
        return passed_opts_.find(opt)->second;

    return "";
}

vector<string> cliparser::get_many(const string &&opt) const
{
    vector<string> values;

    if (has(opt)) {
        auto range = passed_opts_.equal_range(opt);

        /* Can't we just return {range.first, range.second} somehow? */
        for (auto &opt = range.first; opt != range.second; opt++)
            values.push_back(opt->second);
    }

    return values;
}

void cliparser::process_arguments(const vector<string> &args)
{
    bool skip_next_arg = false;
    for (size_t i = 0; i < args.size(); i++) {
        if (skip_next_arg) {
            skip_next_arg = false;
            continue;
        }

        const string_view &arg = args[i];
        const string_view &next_arg = args.size() > i + 1 ? args[i + 1] : "";

        skip_next_arg = parse_pair(arg, next_arg);
    }
}

void cliparser::validate_arguments() const
{
    /* Was --ident passed? */
    const bool ident_passed = [opts = passed_opts_] {
        for (const auto &opt : opts) {
            /* .first is the flag. */
            if (opt.first == "ident") return true;
        }

        return false;
    }();

    /* Did we get at least one of the required main flags? */
    const bool main_opt_passed = [opts = passed_opts_, main_opts = valid_groups_[main].options] {
        /* Yeah, a bit of ineffective copying here, but hey, it works. */
        vector<string> required_opts, passed_opts;

        /* Since we only want to match against the long flags, we copy those. */
        for (const auto &opt : opts)
            passed_opts.emplace_back(opt.first);
        for (const auto &opt : main_opts)
            required_opts.emplace_back(opt.flag_long.substr(2));

        return utils::any_intersection(passed_opts, required_opts);
    }();

    if (ident_passed && passed_opts_.size() > 1)
        throw argument_error("ident flag is exclusive and may not be passed with another flag");
    else if (!main_opt_passed)
        throw argument_error("at least one main argument must be specified");

    if (!has(0))
        throw argument_error("you must specify a download path");

    if (positional_args_.size() > 1)
        throw argument_error("only one positional argument is allowed");
}

bool cliparser::parse_pair(const string_view &input, const string_view &input_next)
{
    /*
     * An option with a required value must of course have one, and
     * if a list of valid values are associated with that option,
     * we check that too.
     */
    const auto validate_opt_value = [](const string_view &flag, const string_view &value,
            const vector<string> &valid_values) {
        if (value.empty())
            throw value_error("missing value for " + string(flag.data()));

        if (!valid_values.empty() &&
            std::find(valid_values.cbegin(), valid_values.cend(), value) == valid_values.cend()) {
            throw value_error(
                "invalid value '" + string(value.data()) + "' for argument " + string(flag.data()) +
                "; valid options are: " + utils::vector_to_string(valid_values)
            );
        }

        return value;
    };

    /* An option must either match its long of short variant to exist. */
    const auto opt_exists = [](const string_view &option, string opt_short, string opt_long) {
        const bool is_short = option.compare(0, opt_short.length(), opt_short) == 0,
                   is_long  = option.compare(0, opt_long.length(), opt_long) == 0;

        return is_short || is_long;
    };

    for (const auto &group : valid_groups_) {
        for (const auto &opt : group.options) {
            if (opt_exists(input, opt.flag, opt.flag_long)) {
                if (opt.token.empty()) {
                    /* The option is only a flag. */
                    passed_opts_.emplace(opt.flag_long.substr(2), "");
                    return false;
                }

                /*
                 * The option should have an accompanied value.
                 * Let's verify it with opt.values.
                 */
                const auto value = validate_opt_value(input, input_next, opt.values);
                passed_opts_.emplace(opt.flag_long.substr(2), value);

                return value == input_next;
            }
        }
    }

    if (input[0] == '-')
        throw argument_error("unrecognized option " + string(input.data()));

    positional_args_.emplace_back(input);
    return false;
}
