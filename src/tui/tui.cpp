#include <iostream>

#include "tui.hpp"
#include "curses_wrap.hpp"

namespace bookwyrm::tui {

colour to_colour(core::log_level lvl)
{
    using level = core::log_level;

    switch (lvl) {
        case level::debug:
            return colour::blue;
        case level::warn:
            return colour::yellow;
        case level::err:
        case level::critical:
            return colour::red;
        default:
            return colour::none;
    }
}

logger::~logger()
{
    std::lock_guard<std::mutex> guard(log_mutex_);

    for (const auto& [lvl, msg] : buffer_)
        (lvl <= core::log_level::warn ? std::cout : std::cerr) << msg << "\n";
}

void logger::log(const core::log_level level, std::string message)
{
    if (level < wanted_level_)
        return;

    std::lock_guard<std::mutex> guard(log_mutex_);

    if (screen_.expired()) {
        buffer_.emplace_back(level, message);
    } else if (const auto screen = screen_.lock(); is_log_focused()) {
        screen->log_entry(level, message);
    } else {
        buffer_.emplace_back(level, message);
    }
}

core::log_level logger::worst_unread() const
{
    if (!has_unread_logs())
        throw std::runtime_error("__func__: buffer is empty");

    const auto worst = std::max_element(cbegin(buffer_), cend(buffer_),
        [] (const buffer_pair &a, const buffer_pair &b) {
            return a.first < b.first;
        });

    return worst->first;
}

void logger::flush_to_screen()
{
    if (screen_.expired())
        throw std::runtime_error("__func__: buffer is empty");

    std::lock_guard<std::mutex> guard(log_mutex_);

    const auto screen = screen_.lock();
    for (const auto& [level, message] : buffer_)
        screen->log_entry(level, message);

    buffer_.clear();
}

tui::tui(std::set<core::item> &items, bool debug_log)
    : viewing_details_(false), items_(items)
{
    /* Create the log screen. */
    log_ = std::make_shared<screen::log>();

    /* And create the default menu screen and focus on it. */
    index_ = std::make_shared<screen::multiselect_menu>(items_);
    focused_ = index_;

    /* Create the logger. */
    logger_ = std::make_unique<logger>(log_,
            (debug_log ? core::log_level::debug : core::log_level::warn),
            [this]() { return this->is_log_focused(); });

    logger_->debug("the mighty bookwyrm hath been summoned!");
}

void tui::update()
{
    repaint_screens();
}

bool tui::is_log_focused() const
{
    return focused_ == log_;
}

void tui::log(const core::log_level level, const std::string message)
{
    /* XXX: ugly; rethink this. */
    switch (level) {
        case core::log_level::trace:
            logger_->trace(message);
            break;
        case core::log_level::debug:
            logger_->debug(message);
            break;
        case core::log_level::info:
            logger_->info(message);
            break;
        case core::log_level::warn:
            logger_->warn(message);
            break;
        case core::log_level::err:
            logger_->err(message);
            break;
        case core::log_level::critical:
            logger_->critical(message);
            break;
        default:
            return;
    }

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
    const auto print_right_align = [this](int y, std::string &&str, const colour attrs = colour::none) {
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
                to_colour(logger_->worst_unread()) | attribute::reverse | attribute::bold);
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
        const key ch = static_cast<key>(curses::getkey());

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

std::vector<core::item> tui::get_wanted_items()
{
    std::vector<core::item> items;

    for (int idx : index_->marked_items())
        items.push_back(*std::next(items_.cbegin(), idx));

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
            if (is_log_focused()) return false;
            return open_details();
        case 'h':
        case key::arrow_left:
            if (is_log_focused()) return false;
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

    details_ = std::make_shared<screen::item_details>(index_->selected_item(),
            curses::get_height() - height - 1);
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

void tui::print(int x, const int y, const std::string &str, const colour attrs)
{
    curses::mvprint(x, y, str, attribute::none, attrs);
}

void tui::printcont(int x, const int y, const std::string &str, const colour attrs)
{
    curses::mvprint(x, y, str, attribute::none, attrs);

    for (int i = x + str.length(); i < curses::get_width(); i++)
        curses::mvprint(i, y, " ", attribute::none, attrs);
}

/* ns bookwyrm::tui */
}
