#pragma once

namespace bookwyrm::core::algorithm {

/*
 * Generates the cartesian product between two vectors.
 */
template <class T1, class T2>
auto product(const vector<T1> &a, const vector<T2> &b)
{
    /* NOTE: are we copying the references or the full strings here? */
    vector<std::pair<T1, T2>> products;

    for (const auto &first : a) {
        for (const auto &second : b)
            products.emplace_back(first, second);
    }

    return products;
}

/* ns bookwyrk::core::algorithm */
}
