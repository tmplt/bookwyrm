#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# This file is part of bookwyrm

"""
Scrapes Library Genesis <https://en.wikipedia.org/wiki/Library_Genesis> for item matches.
"""

# Library Genesis is dirty to parse. Consequently, this file is dirty, ugly, and gross.
# If you are able, and have some spare time, please submit improvements of this file.
#
# TODO list:
#   - parse and process Scimag
#   - parse and process Russian fiction
#   - parse and process comics
#   - parse and process standards
#   - parse and process magazines
#   - Fix all TODO:s

import pybookwyrm as bw
from pdb import set_trace as breakpoint

from pymaybe import maybe
from bs4 import BeautifulSoup
from furl import furl
from collections import deque
from enum import Enum
import sys
import requests
import re
import isbnlib

DOMAINS = ('libgen.io', '93.174.95.27')
DEBUG = __name__ == '__main__'

class FakeLogger():
    def __getattr__(self, attr):
        return lambda msg: print(msg)


class FakeBookwyrm():
    def __getattr__(self, attr):
        if attr == "log":
            return FakeLogger()
        elif attr == "feed":
            return lambda item: print(item)

#
# Utility functions
# General enough to be on their own.
#

def translate_size(string):
    """
    Translate a size on the string form '1337 kb' and similar to a number of bytes.
    """
    try:
        count, unit = string.split(' ')
        count = int(count)
    except (ValueError, TypeError):
        return

    si_prefix = {
        'k': 1e3,
        'K': 1e3,
        'M': 1e6,
        'G': 1e9
    }

    # While LibGen lists sizes in '[KM]b', it's actually in bytes (B)
    return int(count * si_prefix[unit[0]])


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def debuginfo(msg):
    global DEBUG
    if DEBUG:
        eprint(f"\033[93m\tD: {msg}\033[0m")


class LibgenSeeker(object):
    def __init__(self, wanted, bookwyrm=FakeBookwyrm()):
        self.queries = self.build_queries(wanted)
        self.bookwyrm = bookwyrm

        debuginfo(self.queries)

    def log(self, level, msg):
        if self.bookwyrm:
            self.bookwyrm.log(level, 'from ' + __file__ + ': ' + msg)
        else:
            print(msg)

    def feed(self, item):
        """
        Feed an item to the bookwyrm instance, otherwise print it out.
        """
        item = {**item, 'origin_plugin': 'libgen.py'}

        if self.bookwyrm:
            self.bookwyrm.feed(item)
        else:
            print(item)

    def build_queries(self, item):
        """
        Build a set of Library Genesis search queries from an item.
        The set contains a pair: (path to index.php, item dictionary)
        """

        # Library Genesis divides its library into the following categories:
        #   * LibGen (Sci-Tech) -- text books (/search.php);
        #   * Fiction (/foreignfiction/index.php);
        #   * Scientific Articles (/scimag/index.php);
        #   * Russion fiction (/fiction_rus/index.php; different structure; on hold);
        #   * Comics (/comics/index.php);
        #   * Standards (/standarts/index.php; different structure; on hold), and
        #   * Magazines (different structure (domain, et al.; on hold)
        #
        # Categories marked on hold have not been implemented yet.
        #    To get everything we want, we aptly build a set set of queries that
        # searches all categories. We return a set of pairs holding the query dictionary,
        # and the path to the index.php, absolute to domain root.
        #    Unfortunately, Library Genesis only allows to search one field at a time,
        # so when the wanted item specifies title AND authors AND publisher etc.,
        # we must do a query per field. Luckily, bookwyrm does the heavy lifting for us, sorting away
        # unwanted items, and some fields are unecessary, e.g. extension and year;
        # these are not fuzzily matched.

        queries = []

        #
        # The Text Book Category (/search.php)
        #

        # All appended queries below inherit the following keys
        base = {
            'req': 100,       # 100 results per page
            'view': 'simple'  # The HTML we want to parse
        }

        def search_for_in(req, col):
            queries.append(('/search.php', {**base, 'req': req, 'column': col}))

        if 'title' in item:
            search_for_in(item['title'], 'title')

        if 'authors' in item:
            search_for_in(', '.join(item['authors']), 'author')

        if 'series' in item:
            search_for_in(item['series'], 'series')

        if 'publisher' in item:
            search_for_in(item['publisher'], 'publisher')

        #
        # The Fiction Category (/foreignfiction/index.php)
        #

        base = {
            'f_ext': item['extension'] if 'extension' in item else "All",
            'f_group': 0,  # Don't group results of differing extensions.
            'f_lang': 0,   # Search for all languages.
        }

        fields = {'all': 0, 'title': 1, 'authors': 2, 'series': 3}

        def search_for_in(s, col):
            queries.append(('/foreignfiction/index.php', {**base, 's': s, 'f_column': col}))

        if 'title' in item:
            search_for_in(item['title'], fields['title'])

        if 'authors' in item:
            search_for_in(', '.join(item['authors']), fields['authors'])

        if 'series' in item:
            search_for_in(item['series'], fields['series'])

        #
        # The Scientific Articles Category
        #

        # def non_empty(*xs):
        #     res = [x for x in xs if x != bw.empty]
        #     return res[0] if res else None

        # queries.append(
        #     ('/scimag/index.php', {
        #         's': ne.title,
        #         'journalid': ne.journal,
        #         'v': non_empty(e.volume, e.year),  # TODO: e.volume should be a string?
        #         # 'i': add ne.issue?
        #         'p': non_empty(e.pages),
        #         'redirect': 0  # Don't redirect to Sci-Hub on no results
        #     })
        # )

        return queries


    def search(self):
        global DOMAINS

        for query in self.queries:
            for domain in DOMAINS:
                path, params = query
                f = furl('http://' + domain + path).set(query_params=params)

                try:
                    for table in self.tables_fetcher(f):
                        if path == '/search.php':
                            self.process_libgen(table)
                        elif path == '/foreignfiction/index.php':
                            self.process_ffiction(table)
                        else:
                            self.bookwyrm.log.warn('unknown path "%s"; ignored.' % path)
                except requests.exceptions.ConnectionError as e:
                    self.bookwyrm.log.error('connection error (%s)!' % e)
                    continue
                except requests.exceptions.HTTPError as e:
                    self.bookwyrm.log.error('HTTP error (%s)!' % e)
                    continue

                # That domain worked; do the next query.
                break


    def tables_fetcher(self, f):
        """
        A generator that given a start URL, fetches each page of result,
        by supplying a page=n parameter.
        Yields a soup of the result table.
        """

        # Both /search.php and /foreignfiction/index.php uses some JavaScript under the name
        # Paginator 3000 to generate the scrollbars that displays the pages available. The
        # current page can easily be extracted to create a loop invariant, were we to actually
        # execute JS.
        #     With the most logical way to check if we're on the last page out of the window,
        # we'll have to hack some conditions together:
        #     - /search.php: check if extracted table only contains a single line;
        #     - /foreignfiction/index.php: check if extracted table only contains a single line;
        # When the respective invariant is True, we've gone through all pages.

        p = 1
        query_params = f.args.copy()

        table_extractors = {
            '/search.php': lambda soup: soup.find('table', {'class': 'c', 'rules': 'rows'}, recursive=False),
            '/foreignfiction/index.php': lambda soup: soup.find_all('table', {'rules': 'rows'})[-1],
        }
        exhaust_conditions = {
            '/search.php': lambda table: len(table.find_all('tr')) == 1,
            '/foreignfiction/index.php': lambda table: len(table.find_all('tr')) == 0
        }

        while True:
            f.set({'page': p}).add(query_params)

            # Fetch the page
            headers = {
                'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64; rv:63.0) Gecko/20100101 Firefox/63.0'
            }
            r = requests.get(f.url, headers=headers)
            if r.status_code != requests.codes.ok:
                r.raise_for_status()

            # Extract table
            soup = BeautifulSoup(r.text, 'html.parser')
            table = table_extractors[str(f.path)](soup)

            # Have we exhaused all pages?
            if exhaust_conditions[str(f.path)](table):
                return

            yield table

            p += 1

    def process_libgen(self, table):
        """
        Processes a table soup from LibGen and returns the items found within.
        """
        # TODO: check if the same HTML is given when using gen.lib.rus.ec host.

        def make_item(row):
            columns = row.find_all('td')

            # In a row, the first column contains the item's ID number on LibGen.
            # The other columns contain a single piece of data, the raw text of which
            # we aptly extract. The third column, however, contains the item's series
            # (in green cursive), title (blue), edition (cursive, green, in brackets),
            # and a number of ISBN numbers (green, cursive).
            #
            # Also, within this STEI- (series, title, edition, isbsns) field, the title,
            # series, edition and isbsns are all in the same <a>-tag. Fortunately,
            # the title is always in normal text, while the latter are both in their own
            # <font>-tags.

            authors, stei, publisher, year, pages, language, \
                size, extension = columns[1:9]
            mirrors = columns[9:-1]  # Last column is a link to edit the entry.

            # If first <a>-tag has title attribute, the item does not have a series.
            has_series = not stei.a.has_attr('title')

            # The title, edition, and isbn section always contain an id-attribute with
            # an integer.
            tei = stei.find_next('a', id=re.compile('\d+$'))

            item = {
                'series': stei.a.text if has_series else '',
                'publisher': publisher.text or None,
                'language': language.text,
                'authors': authors.text.split(', '),
                'pages': maybe(re.search('\d+', pages.text)).group(),
                'size': translate_size(size.text),
                'extension': extension.text,
                'mirrors': [m.a['href'] for m in mirrors if m.a != None]
            }

            def extract_year(year):
                if not year.text:
                    return None

                # Extract the first year found, if any
                year = maybe(re.search('\d+', year.text)).group()
                return int(year) if year else None

            def extract_title():
                try:
                    # The title is the first sibling of the first font tag, if any
                    dd = deque(tei.font.previous_siblings, maxlen=1)
                    return dd.pop()
                except AttributeError:
                    # No edtion or isbn numbers; only the title is given.
                    return tei.text

            def extract_edition():
                # Always surrounded by brackets, so look for those.
                for font in tei.find_all('font', recursive=False):
                    if font.text.startswith('[') and font.text.endswith(']'):
                        return font.text[1:-1].replace('\xa0', ' ')  # replace no-break space

            def extract_isbns():
                def valid_isbn(isbn):
                    return isbnlib.is_isbn10(isbn) or isbnlib.is_isbn13(isbn)

                # Always comma-seperated, so split those and check all elements
                for font in tei.find_all('font', recursive=False):
                    if font.text.startswith('[') and font.text.endswith(']'):
                        # We stumbled upon the edition, again.
                        continue
                    return [isbn for isbn in font.text.split(', ') if valid_isbn(isbn)]


            item = {
                **item,
                'title': extract_title().strip(),
                'edition': extract_edition() or None,
                'authors': authors.text.split(', '),
                'isbns': extract_isbns() or None,
                'year': extract_year(year)
            }

            return item

        # The first row is the columns' headers, so we skip them.
        for row in table.find_all('tr', recursive=False)[1:]:
            self.feed(make_item(row))

    def process_ffiction(self, table):
        """
        Processes a table soup from LibGen and returns the items found within.
        """

        def make_item(row):
            columns = row.find_all('td')

            # esm: extension, size, and mirrors
            authors, series, title, language, esm = columns

            item = {
                'series': series.text or None,
                'title': title.text,
                'language': language.text,
                'extension': maybe(esm.text.split('(')[0])
                    .lower()
                    .strip()
                    .or_else(None),
                'mirrors': [m['href'] for m in esm.find_all('a')],
            }

            def extract_size():
                size = esm.text[esm.text.find('(')+1:esm.text.find(')')]
                size = size.replace('\xa0', ' ')

                if not ' ' in size:
                    # size looks akin to "280kB"; we inject a space between numbers and letters
                    idx = re.compile("[^\W\d]").search(size).start()
                    size = size[:idx] + ' ' + size[idx:]

                return translate_size(size)


            # Transform "<last name>, <first name>" -> "<first name> <last name>" if possible
            def flip_names(name):
                # Some items are listed with a single word in author name
                if ',' not in name:
                    return name

                lastname, firstname = [s.strip() for s in name.split(',')]
                return f"{firstname} {lastname}"

            item = {
                **item,
                'size': extract_size(),
                'authors': [flip_names(a.text) for a in authors.find_all('a')]
            }

            return item

        # The first row is the columns' headers, so we skip them.
        for row in table.find_all('tr')[1:]:
            self.feed(make_item(row))


def find(wanted, bookwyrm):
    LibgenSeeker(wanted, bookwyrm).search()


# TODO: make this into a neat wrapper, passing the soup as argument
def get_soup(url):
    r = requests.get(url)
    r.raise_for_status()
    return BeautifulSoup(r.text, 'html.parser')


def resolve(mirror):
    def resolve_libgenpw(mirror):
        # Input URL is some item ID (e.g., <https://libgen.pw/item/detail/id/426147>)
        # where the final URL seem to contain some UID in hexadecimal.
        # (e.g., <https://libgen.pw/download/book/5a1f04993a044650f501160e>).
        # Can we derive this number?
        soup = get_soup(mirror)
        uid = soup.find('div', {'class': 'book-info__download'}).a['href'].split('/')[-1]
        return ('https://libgen.pw/download/book/' + uid, {})


    resolvers = {
        'libgen.pw': resolve_libgenpw,
    }

    f = resolvers.get(furl(mirror).host)
    return f(mirror) if f else None


if __name__ == "__main__":
    item = {
        'title': 'Victory of Eagles',
        'series': 'Temeraire',
        'authors': ['Naomi Novik']
    }

    try:
        LibgenSeeker(item).search()
    except KeyboardInterrupt:
        sys.exit(-1)
