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

/*
 * Take two containers, zip over them and return a vector with the length of the shortest passed container.
 *
 * NOTE: This is some ugly template programming. Please fix it.
 */
template <typename Cont1, typename Cont2>
auto zip(const Cont1 &ac, const Cont2 &bc) -> std::vector<std::pair<typename Cont1::value_type, typename Cont2::value_type>>
{
    std::vector<std::pair<typename Cont1::value_type, typename Cont2::value_type>> pairs;
    auto a = std::cbegin(ac);
    auto b = std::cbegin(bc);

    while (a != std::cend(ac) && b != std::cend(bc))
        pairs.emplace_back(*a++, *b++);

    assert(pairs.size() == std::min(ac.size(), bc.size()));

    return pairs;
}

/* Return true if any element is shared between two sets. */
template <typename Set>
inline bool any_intersection(const Set &a, const Set &b)
{
    return std::find_first_of(a.cbegin(), a.cend(), b.cbegin(), b.cend()) != a.cend();
}

/* Check if the path is a valid download directory. */
std::error_code validate_download_dir(const fs::path &path);

string vector_to_string(const vector<string> &vec);
vector<string> split_string(const string &str);
std::pair<string, string> split_at_first(const string &str, string &&sep);

/* Check if the given path is a file and can be read. */
bool readable_file(const fs::path &path);

/* Returns the ratio of a into b in percentage. */
int ratio(double a, double b);

/* Translates a level enum to a matching colour. */
tui::colour to_colour(core::log_level lvl);

/* Generate a Lorem Ipsum string. */
string lipsum(int repeats);

const core::item create_item(const cliparser &cli);

/* ns utils */
}
