#pragma once
#include <string>
#include <vector>
#include "../3rdparty/cxxopts.hpp"

namespace bw {

struct mirror_t {
    int a;
    std::string b;
    /* ... */
};

struct item_t {
    /* Exacts, these are diff'd without the usage of fuzzy. */
    unsigned int year    = 0,
                 edition = 0,
                 volume  = 0,
                 number  = 0,
                 pages   = 0;

    std::string  lang = "",
                 ext  = "";

    /* Non-exacts, these _are_ diff'd with fuzzy. */
    std::vector<std::string> authors;

    std::string title     = "",
                serie     = "",
                publisher = "",
                journal   = "";

    /* Misc. */
    std::vector<std::string> isbns;
    std::vector<mirror_t>    mirrors;

    item_t(const cxxopts::Options &options)
    {
        /* There must be a better way... */

        /* Exacts: */
        year    = options["year"].as<int>();
        edition = options["edition"].as<int>();
        volume  = options["volume"].as<int>();
        /* number_  = options["number"].as<int>(); */
        /* pages_   = options["pages"].as<int>(); */


        lang = options["lang"].as<std::string>();
        ext  = options["ext"].as<std::string>();

        /* Non-exacts: */
        authors = options["authors"].as<std::vector<std::string>>();

        title     = options["title"].as<std::string>();
        serie     = options["serie"].as<std::string>();
        publisher = options["publisher"].as<std::string>();
        journal   = options["journal"].as<std::string>();

        /* Misc.: */
        isbns   = options["isbns"].as<std::vector<std::string>>();
    }
};

/* ns bw */
}
