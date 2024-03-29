📜 Bookwyrm
---

**This project has been archived:** due to the moving-target nature of ebook sources and a lost interest in the utility I will no longer maintain the code base as it is presented here. Consider this repository abondonware.

---

bookwyrm(1) is a ncurses utility for downloading ebooks and other publications that are available on the Internet.
Given some input data via command line options, bookwyrm will search for any matches and present them to you in a text user interface.
Items are found by the help of plugins that parses sources of these items. Currently, the following sources are queried:
* Library Genesis

Selected items can be viewed for details, printing all bookwyrm knows about the item. When it is implemented,
additional details will be fetched from some external database (unless the source itself holds enough data to satisfy),
such as the Open Library or WorldCat.

Both bookwyrm and plugins may print logs during run-time. These can be viewed by pressing TAB.
All unread logs are printed to stderr upon program termination.

For example, one might run bookwyrm as follows:

    $ bookwyrm --title "discrete mathematics" --extension pdf

[![asciicast](https://asciinema.org/a/248462.png)](https://asciinema.org/a/248462)

# Dependencies

Aside from a C++17-compliant compiler and CMake 3.4.3, bookwyrm depends on a few libraries:
* [fmtlib](http://fmtlib.net/latest/index.html), for a lot of string formatting;
* **ncurses**, for the TUI;
* [pybind11](https://github.com/pybind/pybind11), for Python plugins, and
* **[fuzzywuzzy](https://github.com/Tmplt/fuzzywuzzy)**, for fuzzily matching found items with what's wanted.
* **Python 3**, for Python plugin support.
* **libcurl**, for downloading items over HTTP.

All libraries that are not in bold font are non-essential and may be subject to removal later in development.
Some dependencies are submodules in `lib/`; external dependencies are **ncurses**, **Python 3** and **libcurl**.

Furthermore, the available Python plugins depend on some external packages.
These are listed in [etc/requirements.txt](etc/requirements.txt).

If you're using Nix(OS), all dependencies are declared in [etc/default.nix](etc/default.nix).
A suitable development environment can be generated by sourcing [etc/shell.nix](etc/shell.nix) with nix-shell(1).

# Building instructions

```sh
$ git clone https://github.com/tmplt/bookwyrm.git && cd bookwyrm
$ git submodule update --init --recursive
$ mkdir build && cd build
$ cmake .. && make
```
