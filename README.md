What is this?
---
bookwyrm is a cli-based program written in C++17 which, given some input data,
searches for matching books and academic papers on various sources.
When done, all found items are (will be) presented in a menu ála mutt where you can select which items you want to download.

Planned sources are:
* Library Genesis,
* Sci-Hub,
* various IRC channels (e.g. #ebookz on irc-highway),
* various trackers (incl. private trackers) and
* possibly more.

bookwyrm is a work-in-progress.

Having just found out about [Open Library](https://openlibrary.org/), I think bookwyrm might query it to get as much info as possible regarding the wanted item.
ISBNs, OverDrive IDs and in many cases the actual books themselves are provided by the library.
There is also a (RESTful) API available.

Regarding sources, the inital plan was to write these traversing of these in C++,
but since there are a lot of potential sources it may be a better idea to script these.
So instead of compiling all this,
we could store scripts in `/etc/bookwyrm/sources/`, allow users to create their own in `~/.config/bookwyrm/sources/`,
and keep the "front-end" in C++, to which the scripts feeds the found items.
Doing this, adding support for sources would most likely be faster, easier, and save the eventual overhead of doing it in C++ (efficiency, library porting, etc.).

So, script languages? Python? Support for more?

Also: use [pybind11](https://github.com/pybind/pybind11) for interfacing with Python.

Roadmap
---
- [x] command line parsing
    - [ ] cxxopts-style `.as<T>()` member funtion
- [x] logging
    - [x] log sink that prints to stderr and stdout
- [x] `item` class and data structure
    - [x] fuzzy matching for nonexact data

listed from high to low priority:
- [ ] sources
    - [ ] Open Library (highest priority)
    - [ ] OverDrive
    - [ ] University libraries?
    - [ ] Library Genesis
    - [ ] Sci-Hub
    - [ ] IRC
    - [ ] LibriVox (or would that be outside the scope of the project?)
- [ ] terminal-based gui ala mutt

---

**Reporting Bugs**

Please open a [GitHub Issue](https://github.com/Tmplt/bookwyrm/issues) and include as much information as you can.

Building
---
A compiler that supports C++17 features is required, as `std::experimental::string_view` is used throughout the program.
CMake 3.0 or above is also required.

At present, bookwyrm defaults to build in Debug-mode.
If you wish to build a release executable, pass `-DCMAKE_BUILD_TYPE=Release` to `cmake`.

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```
