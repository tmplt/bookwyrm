#include <fmt/format.h>
#include <ncurses.h>

#include "../../string.hpp"
#include "screens/index.hpp"
#include "screens/item_details.hpp"

namespace bookwyrm::tui::screen {

    bool index::column_t::operator==(const column_t &other) const
    {
        return std::tie(title, width_w, width, startx) == std::tie(other.title, other.width_w, other.width, other.startx);
    }

    index::index(std::set<core::item> const &items)
        : base(default_padding_top, default_padding_bot, default_padding_left, default_padding_right), selected_item_(0),
          scroll_offset_(0), items_(items)
    {
        /*
         * For an example 100px wide window:
         *   35 + 4 + 15 + 25 + 15 + 6 = 100
         */
        columns_ = {{
            {"Title", .35, 0, 0},
            {"Year", 4, 0, 0},
            {"Series", .15, 0, 0},
            {"Authors", .25, 0, 0},
            {"Publisher", .15, 0, 0},
            {"Format", 6, 0, 0},
        }};

        update_column_widths();
    }

    void index::paint()
    {
        erase();

        for (const auto &column : columns_) {
            print_header(column);
            print_column(column);
        }

        refresh();
    }

    std::string index::footer_info() const
    {
        if (plugin_count_ == 0) {
            return fmt::format("Search finished: I found {} items.", item_count());
        } else {
            return fmt::format(
                "Searching with {} running plugins... I have found {} items thus far.", plugin_count_, item_count());
        }
    }

    std::string index::controls_legacy() const { return "[j/k d/u G/g]Navigation [SPACE]Toggle select [l/->]Open details"; }

    int index::scrollpercent() const
    {
        if (item_count() <= capacity())
            return scroll::not_applicable;

        return ratio(selected_item_, item_count());
    }

    void index::prepare(int plugin_count) { plugin_count_ = plugin_count; }

    bool index::is_marked(const size_t idx) const { return marked_items_.find(idx) != marked_items_.cend(); }

    size_t index::capacity() const { return get_height() - 1; }

    void index::move(move_direction dir)
    {
        const bool at_first_item = selected_item_ == 0, at_last_item = selected_item_ == (item_count() - 1),
                   head_selected = selected_item_ == scroll_offset_,
                   tail_selected = selected_item_ == (capacity() - 1 + scroll_offset_);

        switch (dir) {
        case up:
            if (at_first_item)
                return;
            if (head_selected)
                scroll_offset_--;
            selected_item_--;
            break;
        case down:
            if (at_last_item)
                return;
            if (tail_selected)
                scroll_offset_++;
            selected_item_++;
            break;
        case top:
            scroll_offset_ = selected_item_ = 0;
            break;
        case bot:
            selected_item_ = item_count() - 1;
            scroll_offset_ = selected_item_ - capacity() + 1;
            break;
        }
    }

    void index::toggle_action()
    {
        /* Toggle item selection. */
        if (is_marked(selected_item_))
            marked_items_.erase(selected_item_);
        else
            marked_items_.insert(selected_item_);
    }

    void index::update_column_widths()
    {
        size_t x = 1; // x = 0 used by selection marker
        for (auto &column : columns_) {
            try {
                column.width = std::get<int>(column.width_w);
            } catch (std::bad_variant_access &) {
                /* It's a ratio, so multiply it with the full width. */
                const int width = get_width() - 1;
                column.width = width * std::get<double>(column.width_w);
            }

            column.startx = x;
            x += column.width + 3; // We want a 1 char padding on both sides of the seperator.
        }
    }

    void index::on_resize()
    {
        base::on_resize();

        update_column_widths();

        /* Ensure we don't select an item past the tail. */
        if (auto tail = scroll_offset_ + capacity() - 1; selected_item_ > tail)
            selected_item_ = tail;
    }

    void index::print_header(const column_t &col)
    {
        int x = 0;

        /* Center the title. */
        print(col.startx + col.width / 2 - col.title.length() / 2, 0, col.title, attribute::bold, colour::blue);
        x += std::max(col.width, col.title.length());

        /* Print the seperator; account for padding */
        print(col.startx + x + 1, 0, rune::separator);
    }

    void index::print_column(const column_t &col)
    {
        for (size_t i = scroll_offset_, y = 1; i < item_count() && y <= capacity(); i++, y++) {

            const bool on_selected_item = (y + scroll_offset_ == selected_item_ + 1),
                       on_marked_item = is_marked(y + scroll_offset_ - 1);

            /* Print the indicator, indicating which item is currently selected. */
            if (on_selected_item && on_marked_item) {
                print(0, y, rune::double_right_angle_bracket, attribute::reverse);
            } else if (on_selected_item) {
                print(0, y, rune::double_right_angle_bracket);
            } else if (on_marked_item) {
                print(0, y, " ", attribute::reverse);
            }

            const auto str = std::invoke([&]() {
                const auto item = std::next(items_.cbegin(), i);

                switch (std::find(cbegin(columns_), cend(columns_), col) - cbegin(columns_)) {
                case 0:
                    return item->nonexacts.title;
                case 1: {
                    auto &y = item->exacts.year;
                    return (y == core::empty ? " " : std::to_string(y));
                }
                case 2:
                    return item->nonexacts.series;
                case 3:
                    return vector_to_string(item->nonexacts.authors);
                case 4:
                    return item->nonexacts.publisher;
                case 5:
                    return item->exacts.extension;
                default:
                    assert(false);
                }
            });

            const auto attrs = (on_selected_item || on_marked_item) ? attribute::reverse : attribute::none;

            /* Print the string, check if it was truncated. */
            const int trunc_len = printlim(col.startx, y, str, col.width, attrs);

            /*
             * Fill the space between the two column strings with inverted spaces.
             * This makes the whole line seem selected instead of only the strings.
             *
             * We start at the end of the string, just after the last character (or the
             * '~'), and write until the end of the column, plus seperator and the
             * padding on the right side of it (e.g. up to and including the first char
             * in the next column, hence the magic offset).
             */
            const auto start = col.startx + str.length() - trunc_len, end = col.startx + col.width + 2;
            for (auto x = start; x <= end; x++)
                print(x, y, " ", attrs);
        }
    }

    const std::pair<int, int> index::compress_to(double part)
    {
        const int details_height = capacity() * (1 - part);

        /*
         * Will the detail screen hide the currently highlighted item?
         * How much do we need to scroll if we don't want that to happen?
         */
        const int scroll = std::max<int>(selected_item_ - scroll_offset_ - capacity() + 1, 0);
        scroll_offset_ += scroll;

        return {scroll, details_height - 1};
    }

    void index::decompress(int scroll) { scroll_offset_ -= scroll; }

    const core::item &index::selected_item() const
    {
        return *std::next(items_.cbegin(), static_cast<long int>(selected_item_));
    }

    size_t index::item_count() const { return items_.size(); }

    const std::set<int> &index::marked_items() const { return marked_items_; }

} // namespace bookwyrm::tui::screen
