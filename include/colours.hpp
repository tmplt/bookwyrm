#pragma once

#include <type_traits>

namespace bookwyrm {

/* These colours are only valid for TB_OUTPUT_NORMAL. */
enum class colour : uint16_t {
    // TODO: translate to ncurses
    black   = 0,
    red     = 0,
    green   = 0,
    yellow  = 0,
    blue    = 0,
    magenta = 0,
    cyan    = 0,
    white   = 0,
    none    = 0
};

enum class attribute : uint16_t {
    none      = 0,
    bold      = 0,
    underline = 0,
    reverse   = 0
};

using colour_t = std::underlying_type_t<colour>;
using attr_t = std::underlying_type_t<attribute>;

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

/* A string can have multiple attributes, but only one colour. */
colour operator|(colour lhs, colour rhs) = delete;
colour operator|=(colour lhs, colour rhs) = delete;

/* ns bookwyrm */
}
