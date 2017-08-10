/*
 * Copyright (C) 2017 Tmplt <tmplt@dragons.rocks>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <termbox.h>
#include <type_traits>

/* These colours are only valid for TB_OUTPUT_NORMAL. */
enum class colour : uint16_t {
    black   = TB_BLACK,
    red     = TB_RED,
    green   = TB_GREEN,
    yellow  = TB_YELLOW,
    blue    = TB_BLUE,
    magenta = TB_MAGENTA,
    cyan    = TB_CYAN,
    white   = TB_WHITE,
    none    = 0
};

enum class attribute : uint16_t {
    none      = 0,
    bold      = TB_BOLD,
    underline = TB_UNDERLINE,
    reverse   = TB_REVERSE
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
