#pragma once

#include <type_traits>
#include <ncurses.h>
#include <array>

#include "str_const.hpp"

namespace bookwyrm::tui::rune {

const std::string double_right_angle_bracket = "»",
                  em_dash                    = "—",
                  scrollbar_fg               = "█",
                  scrollbar_bg               = "▒";

}

/* using rune_t = std::underlying_type_t<rune::single>; */
