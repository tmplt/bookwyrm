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
#include "components/script_butler.hpp"

namespace bookwyrm {

class multiselect_menu;
class script_butler;

namespace menu {

std::shared_ptr<multiselect_menu> create(script_butler &s);

/* ns menu */
}

class multiselect_menu : public screen_base, public std::enable_shared_from_this<multiselect_menu> {
public:
    explicit multiselect_menu(vector<item> &items);

    void display();
    void update();

private:
    enum move_direction { top, up, down, bot };

    /* Store data about each column between updates. */
    struct columns_t {

        struct column_t {
            using width_w_t = std::variant<int, double>;

            /*
             * width_w(wanted).
             * How much space does the column want?
             * Can be specified as an absolute value or
             * as a multiplier, e.g. 0.30 for 30% of tb_width().
             */
            width_w_t width_w;

            /* Changes whenever the window dimensions are changed. */
            size_t width, startx;
            string title;
        };

        void operator=(vector<std::pair<string, column_t::width_w_t>> &&pairs)
        {
            int i = 0;
            for (auto &&pair : pairs) {
                columns_[i].width_w = std::get<1>(pair);
                columns_[i++].title = std::get<0>(pair);
            }
        }

        column_t& operator[](size_t i) { return columns_[i]; }
        size_t size() { return columns_.size(); }
        auto begin() { return columns_.begin(); }
        auto end()   { return columns_.end();   }

    private:
        std::array<column_t, 6> columns_;
    } columns_;

    /* Index of the currently selected item. */
    size_t selected_item_;

    /* How many lines have we scrolled? */
    size_t scroll_offset_;

    std::mutex menu_mutex_;
    vector<item> const &items_;

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
     * menu "window"?
     */
    bool menu_at_bot() const
    {
        return selected_item_ == (menu_capacity() - 1 + scroll_offset_);
    }

    /*
     * Is the currently selected item the first one in the
     * menu "window"?
     */
    bool menu_at_top() const
    {
        return selected_item_ == scroll_offset_;
    }

    /* Move up and down the menu. */
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

    /* Returns true if the bookwyrm fits in the current terminal window. */
    bool bookwyrm_fits()
    {
        /*
         * I planned to use the classical 80x24, but this menu is
         * in its current form useable in terminals much smaller
         * than that.
         */
        return get_width() >= 50 && get_height() >= 10;
    }

    void update_column_widths();
    void on_resize();

    void print_scrollbar();
    void print_header();
    void print_column(const size_t col_idx);
};

} /* ns bookwyrm */
