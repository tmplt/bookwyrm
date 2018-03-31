#pragma once

#include <type_traits>
#include <ncurses.h>

#include "str_const.hpp"

namespace rune {

namespace single {
const std::string double_right_angle_bracket = "»",
                  em_dash                    = "—",
                  scrollbar_fg               = "█",
                  scrollbar_bg               = "▒";
}

namespace bar {

namespace unicode {
    constexpr str_const left_border   = u8"\u2595",
                        right_border  = u8"\u258F";

    constexpr std::array<const str_const, 9> fraction = {
            " ",                      /* unused */
            u8"\u258F",               /* ▏ */
            u8"\u258E",               /* ▎ */
            u8"\u258D",               /* ▍ */
            u8"\u258C",               /* ▌ */
            u8"\u258B",               /* ▋ */
            u8"\u258A",               /* ▊ */
            u8"\u2589",               /* ▉ */
            u8"\u2588"                /* █ */
    };

}

    constexpr str_const left_border   = "[",
                        right_border  = "]",
                        empty_fill    = " ",
                        tick          = "=";
}

/* http://ascii-table.com/ansi-escape-sequences.php */
namespace vt100 {
    constexpr str_const bar_colour    = "\033[1;34m",
                        border_colour = "\033[1;37m",
                        reset_colour  = "\033[0m",
                        hide_cursor   = "\033[?25l",
                        show_cursor   = "\033[?25h",
                        erase_line    = "\033[K";
}

}

/* using rune_t = std::underlying_type_t<rune::single>; */
