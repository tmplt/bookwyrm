#pragma once

#include <unistd.h>
#include <algorithm>
#include <system_error>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
using std::string;
using std::vector;

namespace bookwyrm::core::utils {

/* Return true if any element is shared between two sets. */
template <typename Set>
inline bool any_intersection(const Set &a, const Set &b)
{
    return std::find_first_of(a.cbegin(), a.cend(), b.cbegin(), b.cend()) != a.cend();
}

string vector_to_string(const vector<string> &vec);

/* Check if the given path is a file and can be read. */
bool readable_file(const fs::path &path);

/* ns bookwyrm::core::utils */
}
