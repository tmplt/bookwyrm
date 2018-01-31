#include <cerrno>
#include <cmath>

#include "utils.hpp"

namespace core::utils {

string vector_to_string(const vector<string> &vec)
{
    string retstring = "";
    for (const auto &e : vec)
        retstring += e + (e != vec.back() ? ", " : "");

    return retstring;
}

bool readable_file(const fs::path &path)
{
    return fs::is_regular_file(path) && access(path.c_str(), R_OK) == 0;
}

/* ns utils */
}
