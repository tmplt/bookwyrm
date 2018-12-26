Project overview
----------------

bookwyrm is build from a backend, `core/`; a set of frontend, `*ui/`; some plugins, `plugins/`; and some components, `components/`.

Core is the integral part of bookwyrm.
Given at least a title, author, series or publisher in the form of a `bookwyrm::core::item`, the backend propagates your query to all available plugins.
Each plugin will during run-time query their respective source for the item of interest and, via a callback, present the found matches for you.
For a more detailed account of the backend, see `core/README.md`.

At present, only a single frontend is available (but more are planned), one in the form of a text user interface.
This mutt-like interface presents all found items in a list, where an item can be selected for download, or "opened", for extra information on it.
A log is available at all times by pressing TAB; this screen will display all errors/warnings from both bookwyrm itself and all found plugins.
A contextual menu is available at all times at the bottom of the TUI, displaying bindings for all possible actions.
For a more detailed account of the TUI, see `tui/README.md`.

While bookwyrm eventually will support the usage of user plugins (`~/.config/bookwyrm/plugins`),
some will be available upstream.
These expose a very simple API which the plugin handler uses to search for and find wanted items.
For a more detailed account of the available plugins, and the plugin API, see `plugins/README.md`.

Some pieces of code are not bookwyrm-specific, and can thus be used in other code bases.
When this document was written, these components where a command line parser, `components/command_line.*`;
and a HTTP downloader based on libcurl, `components/downloader.*`.
