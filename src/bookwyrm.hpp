#pragma once
#include <vector>
#include "../3rdparty/cxxopts.hpp"
#include "item.hpp"

namespace bw {

class Bookwyrm {
public:
    Bookwyrm(const cxxopts::Options &options);

private:
    cxxopts::Options options_;

    item_t wanted_;
    std::vector<item_t> items_;
};

}
