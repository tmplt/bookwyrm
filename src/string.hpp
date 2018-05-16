#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace bookwyrm {

// TODO: improve this (and with string_views)?

string vector_to_string(const vector<string> &vec);
vector<string> split_string(const string &str);
std::pair<string, string> split_at_first(const string &str, string &&sep);

/* ns bookwyrm */
}
