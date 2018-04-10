#include <cerrno>
#include <cmath>

#include "utils.hpp"

namespace bookwyrm::utils {

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

static inline int percent_round(double d)
{
    return std::round<int>(100 * d);
}

int ratio(double a, double b)
{
    return percent_round(a / b);
}

tui::colour to_colour(core::log_level lvl)
{
    using level = core::log_level;

    switch (lvl) {
        case level::debug:
            return tui::colour::blue;
        case level::warn:
            return tui::colour::yellow;
        case level::err:
        case level::critical:
            return tui::colour::red;
        default:
            return tui::colour::none;
    }
}

/* ns utils */
}
