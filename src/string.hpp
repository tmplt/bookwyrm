#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

namespace bookwyrm {

/**
 * @brief Trims leading and trailing characters from a string
 * @param str String that should be trimmed
 * @param delims List of delimiters to trim from the string (defaults to all whitespace)
 */
string trim(const string &str, const string &delims = " \t\n\r\v");

/**
 * @brief Makes a list of a list of strings
 * @param vec The vector of strings that should be made a list
 */
string vector_to_string(const vector<string> &vec);

/**
 * @brief Splits a string into a vector at whitespaces
 * @param str The string that should be split
 */
vector<string> split_string(const string &str);

/**
 * @brief Splits a string in to at a given seperator
 * @param str The string that should be split
 * @param sep The seperator at which the given string should be split
 */
std::pair<string, string> split_at_first(const string &str, string &&sep);

/* ns bookwyrm */
}
