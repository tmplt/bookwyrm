#include <cerrno>
#include <cmath>
#include <unistd.h>

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

static inline int percent_round(double d)
{
    return std::round<int>(100 * d);
}

int ratio(double a, double b)
{
    return percent_round(a / b);
}

/* ns utils */
}
