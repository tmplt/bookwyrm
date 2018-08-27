#include <cctype>

#include <fuzzywuzzy.hpp>

#include "../string.hpp"
#include "algorithm.hpp"
#include "common.hpp"
#include "item.hpp"

namespace bookwyrm::core {

    size_t item::items_idx = 0;

    static int get_integral(const py::dict &dict, const char *key)
    {
        if (!dict.contains(key))
            return empty;
        return py::int_(dict[key]);
    }

    static string get_string(const py::dict &dict, const char *key)
    {
        if (!dict.contains(key))
            return "";
        return trim(py::str(dict[key]));
    }

    static vector<string> get_vector_string(const py::dict &dict, const char *key)
    {
        if (!dict.contains(key))
            return {{}};

        vector<string> strings;
        for (auto &handle : py::list(dict[key]))
            strings.push_back(trim(py::str(handle)));
        return strings;
    }

    exacts_t::exacts_t(const py::dict &dict)
        : ymod(year_mod::unused), year(get_integral(dict, "year")), volume(get_integral(dict, "volume")),
          number(get_integral(dict, "number")), pages(get_integral(dict, "pages")), size(get_integral(dict, "size")),
          extension(get_string(dict, "extension"))
    {
    }

    nonexacts_t::nonexacts_t(const py::dict &dict)
        : authors(get_vector_string(dict, "authors")), title(get_string(dict, "title")), series(get_string(dict, "series")),
          publisher(get_string(dict, "publisher")), journal(get_string(dict, "journal")),
          edition(get_string(dict, "edition"))
    {
    }

    misc_t::misc_t(const py::dict &dict) : uris(get_vector_string(dict, "uris")), isbns(get_vector_string(dict, "isbns")) {}

    bool exacts_t::operator==(const exacts_t &other) const
    {
        /* extension is checked first, because some sources offer items in multiple
         * formats. */
        return std::tie(extension, ymod, year, volume, number, pages, size) ==
               std::tie(other.extension, other.ymod, other.year, other.volume, other.number, other.pages, other.size);
    }

    bool nonexacts_t::operator==(const nonexacts_t &other) const
    {
        return std::tie(authors, title, series, publisher, journal, edition) ==
               std::tie(other.authors, other.title, other.series, other.publisher, other.journal, other.edition);
    }

    bool misc_t::operator==(const misc_t &other) const { return std::tie(uris, isbns) == std::tie(other.uris, other.isbns); }

    bool item::operator==(const item &other) const
    {
        return std::tie(nonexacts, exacts, misc) == std::tie(other.nonexacts, other.exacts, other.misc);
    }

    bool item::matches(const item &wanted, const unsigned int fuzzy_min) const
    {
        /* Return false if any exact value doesn't match what's wanted. */
        for (const auto & [ req, got ] : func::zip(wanted.exacts.store, this->exacts.store)) {
            if (req != empty && req != got)
                return false;
        }

        if (wanted.exacts.year != empty) {
            const auto &req = wanted.exacts.year, got = this->exacts.year;

            switch (wanted.exacts.ymod) {
            case year_mod::equal:
                if (!(got == req))
                    return false;
                break;
            case year_mod::eq_gt:
                if (!(got >= req))
                    return false;
                break;
            case year_mod::eq_lt:
                if (!(got <= req))
                    return false;
                break;
            case year_mod::gt:
                if (!(got > req))
                    return false;
                break;
            case year_mod::lt:
                if (!(got < req))
                    return false;
                break;
            case year_mod::unused:
                /* Should never happen. */
                assert(false);
            }
        }

        /* Ad-hoc the file type, for now. */
        if (!wanted.exacts.extension.empty()) {
            if (this->exacts.extension != wanted.exacts.extension)
                return false;
        }

        /* Does the item contain a wanted ISBN? */
        if (!wanted.misc.isbns.empty() && !func::any_intersection(wanted.misc.isbns, this->misc.isbns))
            return false;

        const std::array<string, 3> in_result = {{this->nonexacts.title, this->nonexacts.series, this->nonexacts.publisher}},
                                    requested = {
                                        {wanted.nonexacts.title, wanted.nonexacts.series, wanted.nonexacts.publisher}};

        for (const auto & [ req, got ] : func::zip(requested, in_result)) {
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
            unsigned int max_ratio = 0;
            for (const auto & [ req, got ] : algorithm::product(wanted.nonexacts.authors, this->nonexacts.authors)) {
                /*
                 * From some quick testing, it feels like token_set_ratio
                 * works best here.
                 */
                auto ratio = fuzz::token_set_ratio(req, got);
                max_ratio = std::max(ratio, max_ratio);

                if (max_ratio >= fuzzy_min)
                    break;
            }

            if (max_ratio < fuzzy_min)
                return false;
        }

        return true;
    }

} // namespace bookwyrm::core
