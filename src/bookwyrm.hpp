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

class Bookwyrm {
public:
    Bookwyrm(const cxxopts::Options &options);

private:
    /* Exacts, these are diff'd without the usage of fuzzy. */
    unsigned int year_    = 0,
                 edition_ = 0,
                 volume_  = 0,
                 number_  = 0,
                 pages_   = 0;

    std::string  lang_    = "",
                 ext_     = "";

    /* Non-exacts, these _are_ diff'd with fuzzy. */
    std::vector<std::string> authors_;

    std::string title_     = "",
                serie_     = "",
                publisher_ = "",
                journal_   = "";

    /* Misc. */
    std::vector<std::string> isbns_;
    std::vector<mirror_t>    mirrors_;
};

/* ns bw */
}
