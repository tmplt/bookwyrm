#pragma once

#include <cassert>
#include <array>
#include <tuple>
#include <map>

#include "utils.hpp"

using std::string;
using std::vector;

namespace core {

/* Default value: "this value is empty". */
constexpr int empty = -1;

/*
 * For --year:
 *   -y 2157   : list items from 2157 (equal)
 *   -y =>2157 : list items from 2157 and later (eq_gt)
 *   -y =<2157 : list items from 2157 and earlier (eq_lt)
 *   -y >2157  : list items from later than 2157 (gt)
 *   -y <2157  : list items from earlier than 2157 (lt)
 */
enum class year_mod { equal, eq_gt, eq_lt, lt, gt, unused };

struct exacts_t {
    /* Holds exact data about an item (year, page count, format, etc.). */

    explicit exacts_t(std::pair<year_mod, int> yearmod, int volume, int number, const string &extension)
        : ymod(std::get<0>(yearmod)), year(std::get<1>(yearmod)), volume(volume), number(number),
        pages(empty), size(empty), extension(extension) {}

    explicit exacts_t(const std::map<string, int> &dict, const string &extension)
        : ymod(year_mod::unused),
        year(get_value(dict, "year")),
        volume(get_value(dict, "volume")),
        number(get_value(dict, "number")),
        pages(get_value(dict, "pages")),
        size(get_value(dict, "size")),
        extension(extension) {}

    const year_mod ymod;
    const int year,
              volume,  /* no associated flag */
              number,  /* no associated flag */
              pages,   /* no associated flag */
              size;    /* in bytes; no associated flag */

    const string extension;

    /* Convenience container */
    const std::array<int, 5> store = {{
        year, volume, number, pages, year
    }};

private:
    static int get_value(const std::map<string, int> &dict, const string &&key);
};

struct nonexacts_t {
    /* Holds strings, which are matched fuzzily. */

    explicit nonexacts_t(const vector<string> &authors, const string &title, const string &series,
            const string &publisher, const string &journal)
        : authors(authors), title(title), series(series), publisher(publisher), journal(journal) {}

    explicit nonexacts_t(const std::map<string, string> &dict, const vector<string> &authors)
        : authors(authors),
        title(get_value(dict, "title")),
        series(get_value(dict, "series")),
        publisher(get_value(dict, "publisher")),
        journal(get_value(dict, "journal")),
        edition(get_value(dict, "edition")) {}

    const vector<string> authors;
    const string title;
    const string series;
    const string publisher;
    const string journal;
    const string edition;

private:
    static const string get_value(const std::map<string, string> &dict, const string &&key);
};

struct request {
    /* Holds necessary data to download an item. */

    /* some type enum? ´headers´ will only be used when the mirror is over HTTP. */
    const string uri;
    const std::map<string, string> headers;
};

struct misc_t {
    /* Holds everything else. */
    explicit misc_t(const vector<string> &uris, const vector<string> &isbns)
        : uris(uris), isbns(isbns) {}
    explicit misc_t() {}

    const vector<string> uris;
    const vector<string> isbns;
};

class item {
public:

    explicit item(const nonexacts_t ne, const exacts_t e)
        : nonexacts(ne), exacts(e), misc() {}

    /* Construct an item from a pybind11::tuple. */
    explicit item(const std::tuple<nonexacts_t, exacts_t, misc_t> &tuple)
        : nonexacts(std::get<0>(tuple)), exacts(std::get<1>(tuple)), misc(std::get<2>(tuple)) {}

    /*
     * Returns true if all specified exact values are equal
     * and if all specified non-exact values passes the fuzzy ratio.
     */
    bool matches(const item &wanted) const;

    const nonexacts_t nonexacts;
    const exacts_t exacts;
    const misc_t misc;
};

/* ns bookwyrm */
}
