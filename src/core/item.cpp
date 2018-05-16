#include <cctype>

#include <fuzzywuzzy.hpp>

#include "item.hpp"
#include "common.hpp"
#include "algorithm.hpp"
#include "../string.hpp"

namespace bookwyrm::core {

static constexpr int fuzzy_min = 75;

size_t item::items_idx = 0;

int exacts_t::get_value(const std::map<string, int> &dict, const string &&key)
{
    const auto elem = dict.find(key);
    return elem == dict.cend() ? empty : elem->second;
}

const string nonexacts_t::get_value(const std::map<string, string> &dict, const string &&key)
{
    const auto elem = dict.find(key);
    return elem == dict.cend() ? "" : elem->second;
}

bool exacts_t::operator==(const exacts_t &other) const
{
    /* extension is checked first, because some sources offer items in multiple formats. */
    return std::tie(extension, ymod, year, volume, number, pages, size) ==
        std::tie(other.extension, other.ymod, other.year, other.volume, other.number, other.pages, other.size);
}

bool nonexacts_t::operator==(const nonexacts_t &other) const
{
    return std::tie(authors, title, series, publisher, journal, edition) ==
        std::tie(other.authors, other.title, other.series, other.publisher, other.journal, other.edition);
}

bool misc_t::operator==(const misc_t &other) const
{
    return std::tie(uris, isbns) == std::tie(other.uris, other.isbns);
}

bool item::operator==(const item &other) const
{
    return std::tie(nonexacts, exacts, misc) ==
        std::tie(other.nonexacts, other.exacts, other.misc);
}

bool item::matches(const item &wanted) const
{
    /* Return false if any exact value doesn't match what's wanted. */
    for (const auto& [req, got] : func::zip(wanted.exacts.store, this->exacts.store)) {
        if (req != empty && req != got)
            return false;
    }

    /* Ad-hoc the file type, for now. */
    if (!wanted.exacts.extension.empty()) {
        if (this->exacts.extension != wanted.exacts.extension)
            return false;
    }

    /* Does the item contain a wanted ISBN? */
    if (!wanted.misc.isbns.empty() &&
            !func::any_intersection(wanted.misc.isbns, this->misc.isbns))
        return false;

    const std::array<string, 3> in_result = {{
                                    this->nonexacts.title,
                                    this->nonexacts.series,
                                    this->nonexacts.publisher
                                }},
                                requested = {{
                                    wanted.nonexacts.title,
                                    wanted.nonexacts.series,
                                    wanted.nonexacts.publisher
                                }};

    for (const auto& [req, got] : func::zip(requested, in_result)) {
        if (!req.empty()) {
            /*
             * partial: useful for course literature that can have some
             * crazy long titles. Also useful for publishers, because
             * some entries may not use the full name.
             */
            if (fuzz::partial_ratio(got, req) < fuzzy_min)
                return false;
        }
    }

    if (!wanted.nonexacts.authors.empty()) {
        int max_ratio = 0;
        for (const auto& [req, got] : algorithm::product(wanted.nonexacts.authors,
                    this->nonexacts.authors)) {
            /*
             * From some quick testing, it feels like token_set_ratio
             * works best here.
             */
            int ratio = fuzz::token_set_ratio(req, got);
            max_ratio = std::max(ratio, max_ratio);

            if (max_ratio >= fuzzy_min)
                break;
        }

        if (max_ratio < fuzzy_min)
            return false;
    }

    return true;
}

/* ns bookwyrm::core */
}
