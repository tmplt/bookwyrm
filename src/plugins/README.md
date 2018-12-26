# Plugins

## `libgen.py`
Parses the venerable Library Genesis.
No API is available, so the HTML must be parsed, intermediate links followed, and final direct links found.

Queries and parses all categories of the site, i.e. Sci-Tech, Fiction, Comics, etc.

Only the `libgen.io` mirror is parsed for now; the others yield different HTML.

### Dependencies
* bs4 (BeautifulSoup), for HTML parsing;
* furl, for easier URL modifications;
* requests, for fetching pages, and
* isbnlib, for validating ISBN numbers.
