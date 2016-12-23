#include <string>
#include <vector>
#include <iostream>
#include "../3rdparty/cxxopts.hpp"
#include "bookwyrm.hpp"

namespace bw {

Bookwyrm::Bookwyrm(const cxxopts::Options &options)
{
    /* There must be a better way... */

    /* Exacts: */
    year_    = options["year"].as<int>();
    edition_ = options["edition"].as<int>();
    volume_  = options["volume"].as<int>();
    /* number_  = options["number"].as<int>(); */
    /* pages_   = options["pages"].as<int>(); */


    lang_ = options["lang"].as<std::string>();
    ext_  = options["ext"].as<std::string>();

    /* Non-exacts: */
    authors_ = options["authors"].as<std::vector<std::string>>();

    title_     = options["title"].as<std::string>();
    serie_     = options["serie"].as<std::string>();
    publisher_ = options["publisher"].as<std::string>();
    journal_   = options["journal"].as<std::string>();

    /* Misc.: */
    /* isbns_   = options["isbns"].as<std::vector<std::string>>(); */
    /* mirrors_ = options["mirrors"].as<std::vector<std::string>>(); */

    std::cout << "check" << std::endl;
}

}
