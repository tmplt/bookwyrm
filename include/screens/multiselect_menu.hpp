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

#include <set>
#include <mutex>
#include <array>
#include <tuple>
#include <utility>
#include <variant>

#include "item.hpp"
#include "screens/base.hpp"

namespace screen {

class multiselect_menu : public base {
public:
    explicit multiselect_menu(vector<bookwyrm::item> const &items);

    void update() override;
    void on_resize() override;
    bool action(const uint16_t &key, const uint32_t &ch) override;

    /*
     * Make some space for a screen:item_details,
     * and return how much we scrolled and how
     * big the new screen will be.
     */
    const std::pair<int, int> compress();

    /* Take back the space lent to screen::item_details */
    void decompress(int scroll);

    const bookwyrm::item& selected_item() const
    {
        return items_[selected_item_];
    }

private:
    /* Store data about each column between updates. */
    struct columns_t {

        struct column_t {
            using width_w_t = std::variant<int, double>;

            string title;

            /*
             * width_w(wanted).
             * How much space does the column want?
             * Can be specified as an absolute value or
             * as a multiplier, e.g. 0.30 for 30% of tb_width().
             */
            width_w_t width_w;

            /* Changes whenever the window dimensions are changed. */
            size_t width, startx;
        };

        /* Initializing the array of columns. */
        void operator=(vector<std::pair<string, column_t::width_w_t>> &&pairs)
        {
            int i = 0;
            for (auto &&pair : pairs) {
                columns_[i].width_w = std::get<1>(pair);
                columns_[i++].title = std::get<0>(pair);
            }
        }

        column_t& operator[](size_t i) { return columns_[i]; }
        size_t size() { return columns_.size();  }
        auto begin()  { return columns_.begin(); }
        auto end()    { return columns_.end();   }

    private:
        std::array<column_t, 6> columns_;
    } columns_;

    /* Index of the currently selected item. */
    size_t selected_item_;

    /* How many lines have we scrolled? */
    size_t scroll_offset_;

    std::mutex menu_mutex_;
    vector<bookwyrm::item> const &items_;

    /* Item indices marked for download. */
    std::set<int> marked_items_;

    size_t item_count() const
    {
        return items_.size();
    }

    bool is_marked(size_t idx) const
    {
        return marked_items_.find(idx) != marked_items_.cend();
    }

    /* How many entries can the menu print in the terminal? */
    size_t menu_capacity() const
    {
        return tb_height() - padding_bot_ - padding_top_;
    }

    /*
     * Is the currently selected item the last one in the
     * menu screen?
     */
    bool menu_at_bot() const
    {
        return selected_item_ == (menu_capacity() - 1 + scroll_offset_);
    }

    /*
     * Is the currently selected item the first one in the
     * menu screen?
     */
    bool menu_at_top() const
    {
        return selected_item_ == scroll_offset_;
    }

    /* Move up and down the menu. */
    enum move_direction { top, up, down, bot };
    void move(move_direction dir);

    /* Select (or unselect) the current item for download. */
    void toggle_select();

    void mark_item(const size_t idx)
    {
        marked_items_.insert(idx);
    }

    void unmark_item(const size_t idx)
    {
        marked_items_.erase(idx);
    }

    void update_column_widths();

    void print_scrollbar();
    void print_header();
    void print_footer();
    void print_column(const size_t col_idx);
};

} /* ns screen */
