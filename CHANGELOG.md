# Changelog

All notable changes to this project will be documented in this file.
This project adheres to [Semantic Versioning](https://semver.org)

## [Unreleased]

### Changed
- Bindings/python: don't add items with a missing/empty title field.
- Bindings/python: feeding an item with `None` in significant fields (e.g. title) no longer throws an exception.
- Plugin handler: improved debug message for ignored items

### Fixed
- `std::out_of_range` exception in `core::item::matches()` for certain string comparisons.

## v0.6.0 - 2018-12-27

* Initial (usable) release

[Unreleased]: https://github.com/Tmplt/bookwyrm/compare/v0.6.0...HEAD
