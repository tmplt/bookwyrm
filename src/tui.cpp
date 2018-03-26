#include "tui.hpp"
#include "utils.hpp"
#include "curses_wrap.hpp"

namespace bookwyrm {

tui::tui(vector<core::item> &items, logger_t logger)
    : items_(items), logger_(logger), viewing_details_(false)
{
    /* Create the log screen. */
    log_ = std::make_shared<screen::log>();

    /* And create the default menu screen and focus on it. */
    index_ = std::make_shared<screen::multiselect_menu>(items_);
    focused_ = index_;
}

void tui::log(const core::log_level level, const string message)
{
    log_->log_entry(level, message);
    repaint_screens();
}

void tui::repaint_screens()
{
    std::lock_guard<std::mutex> guard(tui_mutex_);
    curses::erase();

    if (!bookwyrm_fits()) {
        print(0, 0, "The terminal is too small. I don't fit!");
    } else if (is_log_focused()) {
        log_->paint();
        print_footer();
    } else {
        index_->paint();

        if (viewing_details_)
            details_->paint();

        print_footer();
    }

    curses::refresh();
}

void tui::print_footer()
{
    const auto print_right_align = [this](int y, string &&str, const colour attrs = colour::none) {
        this->print(curses::get_width() - str.length(), y, str, attrs);
    };

    /* Screen info bar. */
    print(0, curses::get_height() - 2, focused_->footer_info());

    /* Scroll percentage, if any. */
    if (int perc = focused_->scrollpercent(); perc > -1)
        print_right_align(curses::get_height() - 2, fmt::format("({}%)", perc));

    /* Screen controls info bar. */
    printcont(0, curses::get_height() - 1, "[q]Quit [TAB]Toggle log " + focused_->controls_legacy(),
            attribute::reverse | attribute::bold);

    /* Any unseen logs? */
    if (logger_->has_unread_logs()) {
        print_right_align(curses::get_height() - 1, " You have unread logs! ",
                utils::to_colour(logger_->worst_unread()) | attribute::reverse | attribute::bold);
    } else {
        print_right_align(curses::get_height() - 1, " You have NO unread logs ", colour::none | attribute::reverse | attribute::bold);
    }
}

void tui::resize_screens()
{
    index_->on_resize();
    log_->on_resize();

    /* Resizing item_details not yet supported. */

    repaint_screens();
}

bool tui::display()
{
    repaint_screens();

    while (true) {
        const key ch = static_cast<key>(getch());

        if (ch == key::resize) {
            close_details();
            resize_screens();
            continue;
        }

        if (ch == 'q')
            return false;

        /* When the terminal is too small, only allow quitting and window resizing. */
        if (!bookwyrm_fits())
            continue;

        if (ch == key::enter)
            return true;

        if (meta_action(ch) || focused_->action(ch))
            repaint_screens();
    }
}

vector<core::item> tui::get_wanted_items()
{
    vector<core::item> items;

    for (int idx : index_->marked_items())
        items.push_back(items_[idx]);

    return items;
}

bool tui::bookwyrm_fits()
{
    return (curses::get_width() >= 50 && curses::get_height() >= 10);
}

bool tui::meta_action(const int ch)
{
    switch (ch) {
        case 'l':
        case key::arrow_right:
            return open_details();
        case 'h':
        case key::arrow_left:
            return close_details();
        case key::tab:
            return toggle_log();
        default:
            return false;
    }
}

bool tui::open_details()
{
    if (viewing_details_ || index_->item_count() == 0) return false;

    /* How much space will the detail menu take up? */
    int height;
    std::tie(index_scrollback_, height) = index_->compress();

    int x, y;
    getmaxyx(stdscr, y, x);

    details_ = std::make_shared<screen::item_details>(index_->selected_item(), y - height - 1);
    focused_ = details_;

    viewing_details_ = true;
    return true;
}

bool tui::close_details()
{
    if (!viewing_details_) return false;

    focused_ = index_;

    /* Give back the space the detail menu too up to the index menu. */
    index_->decompress(index_scrollback_);
    index_scrollback_ = -1;

    viewing_details_ = false;
    return true;
}

bool tui::toggle_log()
{
    if (focused_ != log_) {
        last_ = focused_;
        focused_ = log_;

        logger_->flush_to_screen();
    } else {
        focused_ = last_;
    }

    return true;
}

void tui::print(int x, const int y, const string &str, const colour attrs)
{
    curses::mvprint(x, y, str, attribute::none, attrs);
}

void tui::printcont(int x, const int y, const string &str, const colour attrs)
{
    curses::mvprint(x, y, str, attribute::none, attrs);

    for (int i = x + str.length(); i < curses::get_width(); i++)
        curses::mvprint(i, y, " ", attribute::none, attrs);
}

std::shared_ptr<tui> make_tui_with(core::plugin_handler &plugin_handler, logger_t logger)
{
    plugin_handler.load_plugins();
    auto t = std::make_shared<tui>(plugin_handler.results(), logger);
    plugin_handler.set_frontend(t);
    logger->set_tui(t);
    plugin_handler.async_search();
    return t;
}

/* ns bookwyrm */
}
