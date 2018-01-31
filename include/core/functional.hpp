#pragma once

#include <utility>
#include <vector>
#include <algorithm>

namespace core::func {

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

/* ns func */
}
