#pragma once

#include "common.hpp"

namespace bookwyrm::utils {

string vector_to_string(const vector<string> &vec);
vector<string> split_string(const string &str);
std::pair<string, string> split_at_first(const string &str, string &&sep);

/* ns utils */
}
