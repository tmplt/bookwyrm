# Core

### Description
The core (or backend, if you will) is the integral part of bookwyrm.
Given at the least a title, author, series or publisher, bookwyrm will propagate your query to all available plugins, handled by the aptly named `plugin_handler`.
Each plugin exposes a `find()` function which queries its defining source and gives the result of this back to bookwyrm via function callbacks.
Each plugin also exposes a `resolve(mirror)` function, for resolving the mirrors of a wanted item (getting direct links, setting eventual HTTP headers, etc.).
Each plugin is run in its own thread by calling `async_search()`, and continues to run until the `plugin_handler` is destructed and the program exits;
each worker thread is `std::thread::detach()`ed when it's no longer needed.

`bindings/` contains bindings for the supported plugin languages.
At present, only Python is supported.

All upstream plugins are available in `../plugins/`.
In future releases, users shall be able to write their own plugins into `$XDG_CONFIG_HOME/bookwyrm/`.

### API
bookwyrm's backend exposes the aforementioned `plugin_handler`:
* `plugin_handler(const item &&wanted)`: constructor with the wanted item (provided fields filled in, the rest blank).
* `void load_plugins()`: finds and loads all suitable plugins. Must be called before `async_search()`.
* `void async_search()`: runs each plugin's `find()` function asynchronously.
* `void add_item(py::dict dict)`: add a found item. Never called directly, but bound to Python.
* `void log(log_level lvl, std::string msg)`: log a message from a plugin. Will be used to warn the user about missing/invalid source credentials, for example.
* `std::vector<core::item&> results()`: returns a reference to the vector of all found items.
* `void set_frontend(std::shared_ptr<frontend> fe)`: set which frontend to notify when an item has been found.


### Dependencies
bookwyrm's core depends on
* POSIX Threads, aka pthreads;
* fmt, a modern formatting library ([fmtlib/fmt](https://github.com/fmtlib/fmt));
* fuzzywuzzy, for fuzzy string matching ([Tmplt/fuzzywuzzy](https://github.com/Tmplt/fuzzywuzzy));
* pybind11, for operability to and from Python ([pybind/pybind11](https://github.com/pybind/pybind11)), and
* the C++17 filesystem library, stdc++fs.

pthreads and stdc++fs are required externally; a C++17 compliant compiler is required (I'm using GCC 7.3.0 but I have yet to try Clang).
