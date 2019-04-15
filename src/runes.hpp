#pragma once

#include <ncurses.h>
#include <type_traits>

namespace rune {

    namespace single {
        const std::string double_right_angle_bracket = "»", em_dash = "—";
    }

    namespace bar {

        namespace unicode {
            const std::string left_border = u8"\u2595", right_border = u8"\u258F";

            const std::array<const std::string, 9> fraction = {
                " ",        /* unused */
                u8"\u258F", /* ▏ */
                u8"\u258E", /* ▎ */
                u8"\u258D", /* ▍ */
                u8"\u258C", /* ▌ */
                u8"\u258B", /* ▋ */
                u8"\u258A", /* ▊ */
                u8"\u2589", /* ▉ */
                u8"\u2588"  /* █ */
            };

        } // namespace unicode

        const std::string left_border = "[", right_border = "]", empty_fill = " ", tick = "=";
    } // namespace bar

    /* http://ascii-table.com/ansi-escape-sequences.php */
    namespace vt100 {
        const std::string bar_colour = "\033[1;34m", border_colour = "\033[1;37m", reset_colour = "\033[0m",
                          hide_cursor = "\033[?25l", show_cursor = "\033[?25h", erase_line = "\033[K";
    }

} // namespace rune
