📜 Bookwyrm
---
bookwyrm is a TUI-based program written in C++17 which, given some input data,
searches for matching ebooks and academic papers on various sources.
During runtime, all found items are presented in a menu,
where you can choose which items you want to download.

An item can be viewed for details, which will be fetched from some database (unless the source itself holds enough data), such as the Open Library or WorldCat.
A screen holding logs from worker threads is available by pressing TAB. All unread logs are printed to std{out,err} upon program termination.

For example, one might run bookwyrm as follows:

    bookwyrm --author "Naomi Novik" --series Temeraire --year >=2015 .

Here is an (outdated) video of bookwyrm in action:
[![asciicast](https://asciinema.org/a/9kRtmSvVupD6PsUdtBKQ3vZaD.png)](https://asciinema.org/a/9kRtmSvVupD6PsUdtBKQ3vZaD)

Sources are written as scripts which run in their own worker threads.
Some scripts are available upstream, but you can also write your own into `~/.config/bookwyrm/sources/`. Currently, only Python scripts are supported, but LUA is being considered.
A script may need data from the user (e.g. login credentials); this can be written into `~/.config/bookwyrm/config.yaml`.

Aside from a C++17-compliant compiler and CMake, bookwyrm also depends on a few libraries:
* **fmt**,        for a few print-outs and since spdlog depends on it;
* spdlog,         for logging warnings/errors/etc. to the user;
* termbox,        for the TUI;
* **pybind11**,   for interfacing with Python, and
* **fuzzywuzzy**, for fuzzily matching found items with what's wanted.

All libraries that do not use a bold font are non-essential and may be subject to removal later in development. All dependencies are submoduled in `lib/`.

bookwyrm is still in the early development stage, so nothing is set in stone. A v1.0.0 release is planned for early Q1 2018.

---

Food for thought: the most important part of bookwyrm is her backend, which runs all the plugins.
In the future, bookwyrm will probably be just that; an API will be written so that it should be easy to just add some interface.
It is a very small subset of people that want easier access to books that work in terminals.
Some day, we'll probably have a website for this.

Via the API, everything should be configurable: plugin paths, plugin options (e.g. read from config file by main program), fuzzy match levels, etc.

---

If you have any insights, questions, or wish to contribute,
create a PR/issue on GitLab at <https://gitlab.com/Tmplt/bookwyrm> or Github at <https://github.com/Tmplt/bookwyrm>.
You can also contact me via email at `echo "dG1wbHRAZHJhZ29ucy5yb2Nrcwo=" | base64 -d`.

Building instructions
---
```
$ git submodule update --init --recursive
$ mkdir build && cd build
$ cmake .. && make
$ src/bookwyrm OPTION [OPTION]... PATH
```

If you're using Nix(OS), evaluate the expression in `default.nix` first (append `nix-shell` to the steps above).
