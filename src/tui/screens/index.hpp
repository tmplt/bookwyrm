#pragma once

#include <array>
#include <mutex>
#include <set>
#include <tuple>
#include <utility>
#include <variant>

#include "hash.hpp"
#include "item.hpp"
#include "screens/base.hpp"

namespace bookwyrm::tui::screen {

    class index : public base {
    public:
        explicit index(std::set<core::item> const &items);

        void paint() override;
        void on_resize() override;
        void toggle_action() override;
        void move(move_direction dir) override;
        std::string footer_info() const override;
        std::string controls_legacy() const override;
        int scrollpercent() const override;

        void prepare(int plugin_count);

        /*
         * Make some space for a screen:item_details,
         * and return how much we scrolled and how
         * big the new screen will be.
         */
        const std::pair<int, int> compress_to(double part);

        /* Take back the space lent to screen::item_details */
        void decompress(int scroll);

        const core::item &selected_item() const;

        size_t item_count() const;

        const std::set<int> &marked_items() const;

    private:
        struct column_t {
            using width_w_t = std::variant<int, double>;

            std::string title;

            /*
             * width_w (wanted).
             * How much space does the column want?
             * Can be specified as an absolute value or
             * as a multiplier, e.g. 0.30 for 30% of get_width().
             */
            width_w_t width_w;

            /* Changes whenever the window dimensions are changed. */
            size_t width, startx;

            bool operator==(const column_t &other) const;
            void operator=(std::pair<std::string, width_w_t> &&pair);
        };

        /* Store data about each column between updates. */
        std::array<column_t, 6> columns_;

        /* Index of the currently selected item. */
        size_t selected_item_;

        /* How many lines have we scrolled? */
        size_t scroll_offset_;

        int plugin_count_;

        std::mutex menu_mutex_;
        std::set<core::item> const &items_;

        /* Item indices marked for download. */
        std::set<int> marked_items_;

        bool is_marked(const size_t idx) const;

        /* How many entries can the menu print in the terminal? */
        size_t menu_capacity() const;

        void mark_item(const size_t idx);
        void unmark_item(const size_t idx);

        void update_column_widths();

        int print_header(const column_t &col);
        void print_column(const column_t &col);
    };

} // namespace bookwyrm::tui::screen
