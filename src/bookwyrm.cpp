#include "../3rdparty/cxxopts.hpp"
#include <iostream>
#include "bookwyrm.hpp"

namespace bw {

Bookwyrm::Bookwyrm(const cxxopts::Options &options)
    : options_(options), wanted_(options)
{
    std::cout << wanted_.title << std::endl;
}

}
