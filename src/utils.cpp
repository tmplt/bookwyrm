#include <cerrno>
#include <cmath>

#include "utils.hpp"

namespace utils {

std::error_code validate_download_dir(const fs::path &path)
{
    constexpr auto error = [](auto ec) -> std::error_code {
        return {ec, std::generic_category()};
    };

    if (!fs::exists(path))
        return error(ENOENT);

    if (fs::space(path).available == 0)
        return error(ENOSPC);

    if (!fs::is_directory(path))
        return error(ENOTDIR);

    /* Can we write to the directory? */
    if (access(path.c_str(), W_OK) != 0)
        return error(EACCES);

    return {};
}

string vector_to_string(const vector<string> &vec)
{
    string retstring = "";
    for (const auto &e : vec)
        retstring += e + (e != vec.back() ? ", " : "");

    return retstring;
}

vector<string> split_string(const string &str)
{
    vector<string> tokens;
    string word;

    for (const auto &ch : str) {
        const bool whitespace = std::isspace(ch);

        if (whitespace && word.length() != 0) {
            tokens.push_back(word);
            word.clear();
        } else if (!whitespace) {
            word += ch;
        }
    }

    if (word.size()) {
        tokens.push_back(word);
    }

    return tokens;
}

std::pair<string, string> split_at_first(const string &str, string &&sep)
{
    string left  = str.substr(0, str.find_first_of(sep) + 1),
           right = str.substr(str.find_first_of(sep) + 1);

    return {left, right};
}

bool readable_file(const fs::path &path)
{
    return fs::is_regular_file(path) && access(path.c_str(), R_OK) == 0;
}

/* For testing purposes. */
string lipsum(int repeats)
{
    const string str = "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.";

    string ret;
    for (int i = 0; i < repeats; ++i)
        ret += str + (i == repeats - 1 ? "" : " ");

    return ret;
}

int percent_round(double d)
{
    return std::round<int>(100 * d);
}

int ratio(double a, double b)
{
    return percent_round(a / b);
}

colour to_colour(spdlog::level::level_enum lvl)
{
    using level = spdlog::level::level_enum;

    switch (lvl) {
        case level::debug:
            return colour::blue;
        case level::warn:
            return colour::yellow;
        case level::err: case level::critical:
            return colour::red;
        default:
            return colour::none;
    }
}

const core::item create_item(const cliparser &cli)
{
    const core::nonexacts_t ne(
        cli.get_many("authors"),
        cli.get("title"),
        cli.get("series"),
        cli.get("publisher"),
        cli.get("journal")
    );

    const auto yearmod = [&cli]() -> std::pair<core::year_mod, int> {
        const auto year_str = cli.get("year");
        if (year_str.empty()) return {core::year_mod::equal, core::empty};

        const auto start = std::find_if(year_str.cbegin(), year_str.cend(), [](char c) {
            return std::isdigit(c);
        });

        try {
            /*
             * NOTE: this approach allows the year to be represented as a float
             * (which stoi truncates to an int) and allows appended not-digits.
             * Will this cause problems?
             */
            const auto year = std::stoi(string(start, year_str.cend()));

            if (start != year_str.cbegin()) {
                /* There is a modifier in front of the year. */
                string mod_str(year_str.cbegin(), start);
                core::year_mod mod;

                if (mod_str == "=>")
                    mod = core::year_mod::eq_gt;
                else if (mod_str == "=<")
                    mod = core::year_mod::eq_lt;
                else if (mod_str == ">")
                    mod = core::year_mod::gt;
                else if (mod_str == "<")
                    mod = core::year_mod::lt;
                else
                    throw value_error("unrecognised year modifier '" + mod_str + '\'');

                return {mod, year};
            }

            return {core::year_mod::equal, year};

        } catch (const std::exception &err) {
            throw value_error("malformed year");
        }
    }();

    const auto parse_number = [&cli](const string &&opt) -> int {
        const auto value_str = cli.get(opt);
        if (value_str.empty()) return core::empty;

        try {
            return std::stoi(value_str);
        } catch (std::exception &err) {
            throw value_error("malformed value '" + value_str + "' for argument --" + opt);
        }
    };

    const core::exacts_t e(
        yearmod,
        parse_number("volume"),
        parse_number("number"),
        cli.get("extension")
    );

    const core::item item(std::move(ne), std::move(e));

    return std::move(item);
}

/* ns utils */
}
