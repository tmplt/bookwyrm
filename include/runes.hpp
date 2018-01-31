#pragma once

#include <type_traits>

#include "str_const.hpp"

namespace rune {

enum class single : uint32_t {
    double_right_angle_bracket        = 0xBB,    /* » */
    em_dash                           = 0x2014,  /* — */
    scrollbar_fg                      = 0x2588,  /* █ */
    scrollbar_bg                      = 0x2592,  /* ▒ */
};

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

using rune_t = std::underlying_type_t<rune::single>;
