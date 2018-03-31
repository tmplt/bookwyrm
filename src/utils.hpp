#pragma once

#include <unistd.h>
#include <algorithm>
#include <system_error>
#include <experimental/filesystem>

#include "core/plugin_handler.hpp"
#include "core/item.hpp"
#include "components/command_line.hpp"
#include "common.hpp"
#include "tui/colours.hpp"

namespace fs = std::experimental::filesystem;

namespace bookwyrm::utils {

/* Check if the path is a valid download directory. */
std::error_code validate_download_dir(const fs::path &path);

string vector_to_string(const vector<string> &vec);
vector<string> split_string(const string &str);
std::pair<string, string> split_at_first(const string &str, string &&sep);

/* Check if the given path is a file and can be read. */
bool readable_file(const fs::path &path);

/*
 * Return a rounded percentage in the range [0,100]
 * from a domain of [0.0,1.0]
 */
int percent_round(double d);

/* Returns the ratio of a into b in percentage. */
int ratio(double a, double b);

/* Translates a level enum to a matching colour. */
tui::colour to_colour(core::log_level lvl);

/* Generate a Lorem Ipsum string. */
string lipsum(int repeats);

const core::item create_item(const cliparser &cli);

/* ns utils */
}
