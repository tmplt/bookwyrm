# Changelog

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](https://semver.org)

## [Unreleased]

### Added
- A signal hander. Send `SIGINT` (^C) to prematurely terminate bookwyrm.

### Changed
* Screens/item_details: field "categories" are now separated by an empty line.

### Fixed
* A possible segmentation fault when resizing the terminal.
* Screens/item_details: correctly account for fields spanning multiple lines.
* TUI: possible blank screen until first key input.
* Screens/index: utilize 100% of terminal width.
* Screens/index: possible line overflow.
* Screens/log: segmentation fault when toggling the log in a detached state.
* Screens/log: scroll "jitter" with multi-line entries. #46
* Screens/log: always behave in a "sliding window" manner. #20
* Plugins/libgen: generalized mirror resolution via usage of MD5 value.

### [v0.7.0] - 2019-02-28

### Added
- `bookwyrm --help` now displays the default search accuracy.
- All item information is now printed on the item details screen. #73

### Changed
- bookwyrm no longer displays the TUI unless an item has been found. If no items are found, an appropriate error message is printed, and the program exits with an error. #74

### Fixed
- Program options with the invalid form `-<valid short opt.><random string>` (e.g. `-atest`) are now parsed correctly. #7

## [v0.6.1] - 2019-02-25

### Changed
- Bindings/python: don't add items with a missing/empty title field.
- Plugin handler: improved debug message for ignored items

### Fixed
- `std::out_of_range` exception in `core::item::matches()` for certain string comparisons.
- Bindings/python: feeding an item with `None` in significant fields (e.g. title) no longer throws an exception.
- Bindings/python: feeding an item with a non-integer value in an integer field no longer throws an exception. Corresponding item field is instead set to `core::empty`.

## v0.6.0 - 2018-12-27

* Initial (usable) release

[Unreleased]: https://github.com/Tmplt/bookwyrm/compare/v0.7.0...HEAD
[v0.7.0]: https://github.com/Tmplt/bookwyrm/compare/v0.6.1...v0.7.0
[v0.6.1]: https://github.com/Tmplt/bookwyrm/compare/v0.6.0...v0.6.1
