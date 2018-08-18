#pragma once

#include <cassert>
#include <vector>
#include <string>
#include <array>
#include <algorithm>

#include "python.hpp"
#include "../string.hpp"

using std::string;
using std::vector;

namespace bookwyrm::core {

namespace func {

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

/* ns func */
}

/* Default value: "this value is empty". */
constexpr int empty = -1;

/*
 * For --year:
 *   -y 2157   : list items from 2157 (equal)
 *   -y >=2157 : list items from 2157 and later (eq_gt)
 *   -y <=2157 : list items from 2157 and earlier (eq_lt)
 *   -y >2157  : list items from later than 2157 (gt)
 *   -y <2157  : list items from earlier than 2157 (lt)
 */
enum class year_mod { equal, eq_gt, eq_lt, lt, gt, unused };

struct exacts_t {
    /* Holds exact data about an item (year, page count, format, etc.). */

    explicit exacts_t(std::pair<year_mod, int> yearmod, int volume, int number, const string &extension)
        : ymod(std::get<0>(yearmod)), year(std::get<1>(yearmod)), volume(volume), number(number),
        pages(empty), size(empty), extension(extension) {}

    explicit exacts_t(const py::dict &dict);

    exacts_t()
        : ymod(year_mod::unused),
        year(empty),
        volume(empty),
        number(empty),
        pages(empty),
        size(empty) {}

    bool operator==(const exacts_t &other) const;

    const year_mod ymod;
    const int year,
              volume,  /* no associated flag */
              number,  /* no associated flag */
              pages,   /* no associated flag */
              size;    /* in bytes; no associated flag */

    const string extension;

    /* Convenience container */
    const std::array<int, 5> store = {{
        volume, number, pages
    }};
};

struct nonexacts_t {
    /* Holds strings, which are matched fuzzily. */

    explicit nonexacts_t(const vector<string> &authors, const string &title, const string &series,
            const string &publisher, const string &journal)
        : authors(authors), title(title), series(series), publisher(publisher), journal(journal) {}

    explicit nonexacts_t(const py::dict &dict);

    nonexacts_t() = default;

    bool operator==(const nonexacts_t &other) const;

    const vector<string> authors;
    const string title;
    const string series;
    const string publisher;
    const string journal;
    const string edition;
};

struct request {
    /* Holds necessary data to download an item. */

    /* some type enum? ´headers´ will only be used when the mirror is over HTTP. */
    const string uri;
    const std::map<string, string> headers;
};

struct misc_t {
    /* Holds everything else. */
    explicit misc_t(const py::dict &dict);

    misc_t() = default;

    bool operator==(const misc_t &other) const;

    const vector<string> uris;
    const vector<string> isbns;
};

struct item {
public:
    explicit item(const nonexacts_t ne, const exacts_t e)
        : nonexacts(ne), exacts(e), misc(), index(items_idx++) {}

    explicit item(const py::dict &dict)
        : nonexacts(dict), exacts(dict), misc(dict), index(items_idx++) {}

    item() : index(empty) {}

    /*
     * Returns true if all specified exact values are equal
     * and if all specified non-exact values passes the fuzzy ratio.
     */
    bool matches(const item &wanted, const int fuzzy_min) const;

    bool operator==(const item &other) const;

    /* For keeping sort order in an std::set. */
    bool operator<(const item &other) const
    {
        return index < other.index;
    }

    const nonexacts_t nonexacts;
    const exacts_t exacts;
    const misc_t misc;
    const size_t index;

private:
    static size_t items_idx; // = 0
};

/* ns bookwyrm::core */
}
