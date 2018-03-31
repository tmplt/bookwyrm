#pragma once

#include <type_traits>
#include <ncurses.h>

namespace bookwyrm::tui {

/* Predefined NCurses colour pairs. See screen/base.cpp */
enum class colour : int {
    black   = COLOR_PAIR(1),
    red     = COLOR_PAIR(2),
    green   = COLOR_PAIR(3),
    yellow  = COLOR_PAIR(4),
    blue    = COLOR_PAIR(5),
    magenta = COLOR_PAIR(6),
    cyan    = COLOR_PAIR(7),
    white   = COLOR_PAIR(8),
    none    = 0
};

enum class attribute : int {
    bold      = A_BOLD,
    underline = A_UNDERLINE,
    reverse   = A_REVERSE,
    none      = A_NORMAL
};

using colour_t = std::underlying_type_t<colour>;
using attr_t = std::underlying_type_t<attribute>;

/*
 * OR operations for colours and attributes.
 * Attribues can be OR:ed together, as can a colour with a set of attributes,
 * but two colours cannot.
 */

/* A string can have multiple attributes, but only one colour. */
colour operator|(colour lhs, colour rhs) = delete;
colour operator|=(colour lhs, colour rhs) = delete;

constexpr inline colour operator|(colour lhs, attribute rhs)
{
    return static_cast<colour>(static_cast<colour_t>(lhs) | static_cast<attr_t>(rhs));
}

constexpr inline colour& operator|=(colour &lhs, attribute rhs)
{
    lhs = static_cast<colour>(static_cast<colour_t>(lhs) | static_cast<attr_t>(rhs));
    return lhs;
}

constexpr inline attribute operator|(attribute lhs, attribute rhs)
{
    return static_cast<attribute>(static_cast<attr_t>(lhs) | static_cast<attr_t>(rhs));
}

constexpr inline attribute operator|=(attribute &lhs, attribute rhs)
{
    lhs = static_cast<attribute>(static_cast<attr_t>(lhs) | static_cast<attr_t>(rhs));
    return lhs;
}

constexpr inline attribute operator|(attribute lhs, colour rhs)
{
    return static_cast<attribute>(static_cast<attr_t>(lhs) | static_cast<colour_t>(rhs));
}

constexpr inline attribute operator|=(attribute &lhs, colour rhs)
{
    lhs = static_cast<attribute>(static_cast<colour_t>(lhs) | static_cast<attr_t>(rhs));
    return lhs;
}

/* ns bookwyrm::tui */
}
