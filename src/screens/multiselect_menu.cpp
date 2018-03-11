#include <fmt/format.h>

#include "errors.hpp"
#include "utils.hpp"
#include "screens/multiselect_menu.hpp"
#include "screens/item_details.hpp"

namespace bookwyrm::screen {

void multiselect_menu::columns_t::operator=(vector<std::pair<string, column_t::width_w_t>> &&pairs)
{
    int i = 0;
    for (auto &&pair : pairs) {
        columns_[i].width_w = std::get<1>(pair);
        columns_[i++].title = std::get<0>(pair);
    }
}

multiselect_menu::multiselect_menu(vector<core::item> const &items)
    : base(default_padding_top, default_padding_bot, default_padding_left, default_padding_right),
    selected_item_(0), scroll_offset_(0),
    items_(items)
{
    /*
     * These wanted widths works fine for now,
     * but we are still not utilizing the full 100%
     * of the width, which we should.
     *
     * TODO: Remedy this.
     */
    columns_ = {
        {"Title",      .30},
        {"Year",        4 },
        {"Series",     .15},
        {"Authors",    .20},
        {"Publisher",  .15},
        {"Format",      6 },
    };

    update_column_widths();
}

void multiselect_menu::paint()
{
    for (size_t idx = 0; idx < columns_.size(); idx++) {
        /* Can we fit another column? */
        const size_t allowed_width = get_width() - 1 + padding_left_
                                   - columns_[idx].startx - 2;
        if (columns_[idx].width > allowed_width) break;

        print_column(idx);
    }

    print_header();
}

string multiselect_menu::footer_info() const
{
    return fmt::format("I've found {} items thus far.", item_count());
}

string multiselect_menu::controls_legacy() const
{
    return "[j/k d/u]Navigation [SPACE]Toggle select [l]Open details";
}

int multiselect_menu::scrollpercent() const
{
    if (item_count() <= menu_capacity())
        return scroll::not_applicable;

    return utils::ratio(menu_capacity() + scroll_offset_, item_count());
}

bool multiselect_menu::is_marked(const size_t idx) const
{
    return marked_items_.find(idx) != marked_items_.cend();
}

size_t multiselect_menu::menu_capacity() const
{
    return get_height() - 1;
}

bool multiselect_menu::menu_at_bot() const
{
    return selected_item_ == (menu_capacity() - 1 + scroll_offset_);
}

bool multiselect_menu::menu_at_top() const
{
    return selected_item_ == scroll_offset_;
}

void multiselect_menu::move(move_direction dir)
{
    const bool at_first_item = selected_item_ == 0,
               at_last_item  = selected_item_ == (item_count() - 1);

    switch (dir) {
        case up:
            if (at_first_item) return;
            if (menu_at_top()) scroll_offset_--;
            selected_item_--;
            break;
        case down:
            if (at_last_item) return;
            if (menu_at_bot()) scroll_offset_++;
            selected_item_++;
            break;
        case top:
            scroll_offset_ = selected_item_ = 0;
            break;
        case bot:
            selected_item_ = item_count() - 1;
            scroll_offset_ = selected_item_ - menu_capacity() + 1;
            break;
    }
}

void multiselect_menu::mark_item(const size_t idx)
{
    marked_items_.insert(idx);
}

void multiselect_menu::unmark_item(const size_t idx)
{
    marked_items_.erase(idx);
}

void multiselect_menu::toggle_action()
{
    /* Toggle item selection. */

    if (is_marked(selected_item_))
        unmark_item(selected_item_);
    else
        mark_item(selected_item_);
}

void multiselect_menu::update_column_widths()
{
    size_t x = 1;
    for (auto &column : columns_) {
        try {
            column.width = std::get<int>(column.width_w);
        } catch (std::bad_variant_access&) {
            /* It's a ratio, so multiply it with the full width. */
            const int width = get_width() - 1 + padding_left_;
            column.width = width * std::get<double>(column.width_w);
        }

        column.startx = x;
        x += column.width + 3; // We want a 1 char padding on both sides of the seperator.
    }
}

void multiselect_menu::on_resize()
{
    update_column_widths();

    /*
     * When the window is resized from the lower left/right
     * corner, the currently selected item may escape the
     * menu, so we lock it here.
     *
     * TODO: fix this for cases where the cursor has
     * jumped more than one step down under.
     * (check whether it's just below what's valid, and
     * if so, move it to menu_bot).
     */
    if (menu_at_bot()) selected_item_--;
}

void multiselect_menu::print_header()
{
    /*
     * You might think we should start at x = 0, but that
     * screws up the alignment with the column strings.
     */
    size_t x = 1;
    for (auto &column : columns_) {
        /* Can we fit the next header? */
        const size_t allowed_width = get_width() - 1 + padding_left_
                                   - column.startx - 2;
        if (column.width > allowed_width) break;

        /* Center the title. */
        wprint(x + column.width / 2  - column.title.length() / 2, 0, column.title, colour::blue | attribute::bold);
        x += std::max(column.width, column.title.length());

        /* Padding between the title and the seperator to the left.. */
        x++;

        /* Print the seperator. */
        wprint(x++, 0, "|");

        /* ..and to the right. */
        x++;
    }
}

void multiselect_menu::print_column(const size_t col_idx)
{
    const auto &c = columns_[col_idx];

    for (size_t i = scroll_offset_, y = 1; i < item_count() &&
            y <= menu_capacity(); i++, y++) {

        const bool on_selected_item = (y + scroll_offset_ == selected_item_ + 1),
                   on_marked_item   = is_marked(y + scroll_offset_ - 1);

        /*
         * Print the indicator, indicating which item is
         * currently selected.
         */
        if (on_selected_item && on_marked_item)
            change_cell(0, y, rune::single::double_right_angle_bracket, attribute::reverse);
        else if (on_selected_item)
            change_cell(0, y, rune::single::double_right_angle_bracket);
        else if (on_marked_item)
            change_cell(0, y, ' ', attribute::reverse);

        const attribute attrs = (on_selected_item || on_marked_item) ? attribute::reverse : attribute::none;

        const string authors = utils::vector_to_string(items_[i].nonexacts.authors);
        const string year = std::to_string(items_[i].exacts.year);
        const std::array<std::reference_wrapper<const string>, 6> strings = {{
            items_[i].nonexacts.title,
            year,
            items_[i].nonexacts.series,
            authors,
            items_[i].nonexacts.publisher,
            items_[i].exacts.extension
        }};

        /* Print the string, check if it was truncated. */
        const int trunc_len = wprintlim(c.startx, y, strings[col_idx].get(), c.width, attrs);

        /*
         * Fill the space between the two column strings with inverted spaces.
         * This makes the whole line seem selected instead of only the strings.
         *
         * We start at the end of the string, just after the last character (or the '~'),
         * and write until the end of the column, plus seperator and the padding on the right
         * side of it (e.g. up to and including the first char in the next column, hence the magic).
         */
        const auto string_end = c.startx + strings[col_idx].get().length() - trunc_len,
                   next_start = c.startx + c.width + 2;
        for (auto x = string_end; x <= next_start; x++)
            change_cell(x, y, ' ', attrs);
    }
}

const std::pair<int, int> multiselect_menu::compress()
{
    const int details_height = menu_capacity() * 0.80;
    padding_bot_ = details_height;

    /*
     * Will the detail menu hide the highlighted item?
     * How much do we need to scroll if we don't want that to happen?
     */
    const int scroll = std::max<int>(selected_item_ - scroll_offset_ - menu_capacity() + 1, 0);
    scroll_offset_ += scroll;

    return {scroll, details_height - 1};
}

void multiselect_menu::decompress(int scroll)
{
    padding_bot_ = default_padding_bot;
    scroll_offset_ -= scroll;
}

} /* ns screen */
