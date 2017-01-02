
bookwyrm is a cli-based program written in C++17 which, given some input data,
searches for matching books and academic papers on various sources.
When done, all found items are presented in a menu ála mutt where you can select which items you want to download.

Planned sources are
* Library Genesis,
* Sci-Hub,
* various IRC channels (e.g. #ebookz on irc-highway),
* and various trackers (incl. private trackers).

bookwyrm is a work-in-progress.

---

**Reporting Bugs**

Please open a [GitHub Issue](https://github.com/maciejczyzewski/libchaos/issues) and include as much information as you can.

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
