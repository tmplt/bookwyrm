# Changelog

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](https://semver.org)

## [Unreleased]

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

[Unreleased]: https://github.com/Tmplt/bookwyrm/compare/v0.6.1...HEAD
[v0.6.1]: https://github.com/Tmplt/bookwyrm/compare/v0.6.0...v0.6.1
