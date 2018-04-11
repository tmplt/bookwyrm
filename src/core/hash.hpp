#pragma once

#include "item.hpp"

#include <vector>

/* Hashing functions for bookwyrm::core::item and its fields. */

namespace std
{

/* Source: https://stackoverflow.com/a/27216842, apparently a part of Boost. */
template <class T>
inline void hash_combine(std::size_t & seed, const std::vector<T> &vec)
{
    std::hash<T> hasher;
    for (auto &v : vec) {
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}

/* Source: https://stackoverflow.com/a/9729747, apparently a part of Boost. */
template <class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template <>
struct hash<bookwyrm::core::exacts_t> {
    std::size_t operator()(const bookwyrm::core::exacts_t &e) const
    {
        std::size_t seed = 0;
        hash_combine(seed, e.ymod);
        hash_combine(seed, e.year);
        hash_combine(seed, e.volume);
        hash_combine(seed, e.number);
        hash_combine(seed, e.pages);
        hash_combine(seed, e.size);
        hash_combine(seed, e.extension);
        return seed;
    }
};

template <>
struct hash<bookwyrm::core::nonexacts_t> {
    std::size_t operator()(const bookwyrm::core::nonexacts_t &ne) const
    {
        std::size_t seed = 0;
        hash_combine(seed, ne.authors);
        hash_combine(seed, ne.title);
        hash_combine(seed, ne.series);
        hash_combine(seed, ne.publisher);
        hash_combine(seed, ne.journal);
        hash_combine(seed, ne.edition);
        return seed;
    }
};

template <>
struct hash<bookwyrm::core::misc_t> {
    std::size_t operator()(const bookwyrm::core::misc_t &m) const
    {
        std::size_t seed = 0;
        hash_combine(seed, m.uris);
        hash_combine(seed, m.isbns);
        return seed;
    }
};

template <>
struct hash<bookwyrm::core::item> {
    std::size_t operator()(const bookwyrm::core::item &i) const
    {
        std::size_t seed = 0;
        hash_combine(seed, i.nonexacts);
        hash_combine(seed, i.exacts);
        hash_combine(seed, i.misc);
        return seed;
    }
};


/* ns std */
}
