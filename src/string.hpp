#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace bookwyrm {

// TODO: improve this (and with string_views)?

inline string vector_to_string(const vector<string> &vec)
{
    string retstring = "";
    for (const auto &e : vec)
        retstring += e + (e != vec.back() ? ", " : "");

    return retstring;
}

inline vector<string> split_string(const string &str)
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

inline std::pair<string, string> split_at_first(const string &str, string &&sep)
{
    string left  = str.substr(0, str.find_first_of(sep) + 1),
           right = str.substr(str.find_first_of(sep) + 1);

    return {left, right};
}

/* ns utils */
}
