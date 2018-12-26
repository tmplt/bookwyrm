# TUI

### Description
One of the (hopefully) multiple front-ends to come.
Taking the search query via CLI options, the results given back by the back-end are all presented in an index menu ála mutt(1).
Selected items are put in a `std::vector<core::item>`; the TUI itself does not handle item downloading.

The TUI has a few screen available:
* `screens/base`: a base class, inherited by all other screens (defines some common drawing functions).
* `screens/multiselect_menu`: the index menu where items can be selected.
* `screens/item_details`: displays details about a selected item. The plan is to fetch more data from some database (e.g. WorldCat or Open Library) if the source itself didn't return a good amount.
* `screens/log`: a simple screen that displays all logs made from the running plugins.

A footer is always visible in the TUI, containing a contextual menu.
One line is dedicated for the active screen to print whatever it wants on (extra information, error messages, etc.), along with optional scroll amount.
Another line prints the bindings of the active screen, along with an eventual notification that there are unread logs (cleared after opening the log screen).

### API
The TUI exposes a single class, the `bookwyrm::tui::tui`:
* `void update()` and `void repaint_screens()`: redraws the screen.
* `void log(const core::log_level level, const std::string message)`: add a log entry to the log screen.
* `bool display()`: displays the TUI and takes user input. Returns true if any items are selected for download, otherwise false.
* `std::vector<core::item> get_wanted_items()`: returns an owning vector of the selected items.
* `void print_footer()`: draws the context sensitive TUI footer (explained above).
* `bool is_log_focused()`: returns true if the log screen is focused, otherwise false.

(Yes, the publicity of some of these functions are subject for alterations.
The TUI overall needs some cleanup.)


Along with the above, `std::shared_ptr<tui> make_tui_with(core::plugin_handler &plugin_handler, logger_t logger)` is available for creating the TUI pointer.
The functions parameters are self-explanatory.

### Dependencies
bookwyrm's TUI depends on
* bookwyrm-core (`../core`);
* NCurses

NCurses is an external dependency and must be found by CMAKE's `find_package(Curses)` in order to compile.
