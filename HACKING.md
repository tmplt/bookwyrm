Hacking
-------

bookwyrm is built from a backend (`src/core/` and `include/core/`)
and a frontend (remaining files in `src/` and `include/`).

Given a `core::item`, the backend is responsible for finding items of interest and produce a `request` struct for said item.
This struct contains everything required to download the item with curl. (This struct is not yet implemented.)
The frontend shall handle the actual item downloading.
The backend is compiled as a library and exposes a minimal API:

* `core::item` and its underlying structs — see `include/core/item.hpp`.
* `core::frontend` — a virtual class that must be publically inherited from.
* `core::plugin_handler` — the backend in an object, which exposes the following:
    * `void load_plugins()` — prepares for the search by loading all plugins
    * `void set_frontend(std::shared_ptr<frontend> fe)` — link a frontend to update when finding an item.
    * `vector<core::item>& results()` — returns the vector of all found items. Items will be appended to this vector over time.
    * `void async_search()` — starts the search, one thread for each plugin.

Upon object destruction, all threads used for searching are detached (see #35).

Later on, I'll introduce some configuration struct for backend construction, instead of hardcoding plugin paths, fuzzy string score options, and etc.

At present only a TUI frontend is written, and in a very messy shape.
A complete rewrite into curses is pending.

If you wish to write another frontend, write everything into some `[src,include]/<name>` where `<name>` is cli, gui, for example.
