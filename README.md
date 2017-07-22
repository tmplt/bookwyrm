What is this?
---
bookwyrm is a TUI-based program written in C++17 which, given some input data,
searches for matching books and academic papers on various sources.
When done, all found items are (will be) presented in a menu ála mutt where you can select which items you want to download.

Planned sources are:
* Library Genesis,
* Sci-Hub,
* various IRC channels (e.g. #ebookz on irc-highway),
* various trackers (incl. private trackers) and
* more.

bookwyrm is a work-in-progress, so this document serves as a place to jot down my thought as of now.
Nothing is set in stone for a v1.0.0 release, yet.

How will it work?
---
A "front-end" of sorts will be written in C++.
This will parse command line arguments, sort away unwanted items, and present found items to the user.

The traversal of sources and thus findings items across multiple sources will be written in seperate Python scripts.
When an item is found, it's fed back to the front-end for processing.
The point of this is to make it as easy as possible to add new sources (fit for upstream or not) in some neat path like `~/.config/bookwyrm/sources/`.

While these scripts run, the program will open a menu ála mutt where the list of found items is updated asyncronously.
In this menu items can be reviewed and selected for download.
All info that can be found via a source's main search function (think libgen's) will be seen in this main menu.
Entering an item will make bookwyrm attempt to fetch some more info about the item.
(Some sources like libgen offers more info by just clicking on the item,
but using a database of some sort seems like a better idea.
[Open Library?](https://openlibrary.org/)
[WorldCat?](https://www.worldcat.org/)
Both?)

When it comes to actually download the items I'm not yet sure.
Python's requests module can do this easy peasy, but wouldn't it be better to do that in C++ code?
It'd be a hell of a lot easier if we'd want a progress indicator of some kind.

How does it work thus far?
---
- [x] command line parsing
- [x] logging
    - [x] log sink that prints to stderr and stdout
- [x] `item` class and data structure
    - [x] fuzzy matching for nonexact data (finish [the library](https://github.com/Tmplt/fuzzywuzzy)!)
- [x] embed Python interpreter with [pybind11](https://github.com/pybind/pybind11)
- [x] write a class that handles the plugin threads 
- [ ] write the menu (avoid ncurses, apparently it's a hell to work with)
    - [x] write a simple menu in which we can select multiple entries
    - [ ] write a menu containing item details that overlaps the main menu
- [ ] write scripts for sources (listed in descending priority)
    - [ ] Library Genesis
    - [ ] Sci-Hub
    - [ ] IRC
    - [ ] OverDrive
    - [ ] University libraries?
    - [ ] LibriVox (or would that be outside the scope of the project?)

Here is an (outdated) video of the menu in action:
[![asciicast](https://asciinema.org/a/vLc2A9okFnpLsQ0YKzVATReEt.png)](https://asciinema.org/a/vLc2A9okFnpLsQ0YKzVATReEt)

Building
---
bookwyrm uses some C++17 features such as `std::string_view` and `std::filesystem`,
so a recent compiler is required.
CMake 3.0 or above is also required.

Just clone the repo and:
```
$ mkdir build && cd build
$ cmake ..
$ make
```

Running
---
When compiling in debug, the path for finding Python modules is a hard-coded relative,
so for now:
```
$ cd build
$ src/bookwyrm -a "Naomi Novik" ~/downloads
```
