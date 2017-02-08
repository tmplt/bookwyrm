
bookwyrm is a cli-based program written in C++17 which, given some input data,
searches for matching books and academic papers on various sources.
When done, all found items are presented in a menu ála mutt where you can select which items you want to download.

Planned sources are:
* Library Genesis,
* Sci-Hub,
* various IRC channels (e.g. #ebookz on irc-highway),
* various trackers (incl. private trackers) and
* maybe more.

bookwyrm is a work-in-progress.

Having just found out about [Open Library](https://openlibrary.org/), I think bookwyrm will query it to get as much info as possible. ISBNs, OverDrive IDs and in many cases the actual books themselves are provided by the library. There is also a (RESTful) API available.

Roadmap
---
- [x] command line parsing
    - [ ] cxxopts-style `.as<T>()` member funtion
- [x] logging
    - [x] log sink that prints to stderr and stdout
- [ ] `item` class and data structure
    - [ ] fuzzy matching for nonexact data
- [ ] implement curl

Listed from high to low priority
- [ ] sources
    - [ ] Open Library (highest priority)
    - [ ] OverDrive
    - [ ] University libraries?
    - [ ] Library Genesis
    - [ ] Sci-Hub
    - [ ] IRC
        - [ ] with libircppclient?
- [ ] terminal-based gui ala mutt

---

**Reporting Bugs**

Please open a [GitHub Issue](https://github.com/Tmplt/bookwyrm/issues) and include as much information as you can.

Building
---
A compiler that supports C++17 features is required, as `std::experimental::string_view` is used throughout the program.
CMake 3.0 or above is also required.

At present, bookwyrm defaults to build in Debug-mode.

```
$ mkdir build
$ cd build
$ cmake ..
$ make
```
