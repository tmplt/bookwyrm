#include "string.hpp"

namespace bookwyrm {

    string trim(const string &str, const string &delims)
    {
        size_t b = str.find_first_not_of(delims);
        size_t e = str.find_last_not_of(delims);
        if (b == string::npos || e == string::npos)
            return "";

        return string(str, b, e - b + 1);
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
        string left = str.substr(0, str.find_first_of(sep) + 1), right = str.substr(str.find_first_of(sep) + 1);

        return {left, right};
    }

} // namespace bookwyrm
