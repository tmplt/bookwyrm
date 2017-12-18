#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# This file is part of bookwyrm

# Copyright (C) 2017 Tmplt <tmplt@dragons.rocks>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""
Scrapes Library Genesis <https://en.wikipedia.org/wiki/Library_Genesis> for item matches.
"""

import pybookwyrm as bw
from bs4 import BeautifulSoup
from furl import furl
from collections import deque
import requests
import re
import isbnlib

DOMAINS = ('libgen.io', 'gen.lib.rus.ec')

# TODO list:
#   - fix mirrors for LibGen
#   - parse and process Scimag
#   - parse and process Russian fiction
#   - parse and process comics
#   - parse and process standards
#   - parse and process magazines
#   - Fix all TODO:s

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
        'M': 1e6,
        'G': 1e9
    }

    # While LibGen lists sizes in '[kM]b', it's actually in bytes
    return int(count * si_prefix.get(unit[0]))


#
# Source-specific functions.
#

def build_queries(item):
    """
    Build a set of Library Genesis search queries from an item.
    The set contains a pair: (path to index.php, query dictionary)
    """

    # Library Genesis divides its library into the following categories:
    #   * LibGen (Sci-Tech) -- text books (/search.php);
    #   * Scientific Articles (/scimag/index.php);
    #   * Fiction (/foreignfiction/index.php);
    #   * Russion fiction (/fiction_rus/index.php; different structure; on hold);
    #   * Comics (/comics/index.php);
    #   * Standards (/standarts/index.php; different structure; on hold), and
    #   * Magazines (different structure (domain, et al.; on hold)
    #
    # Categories marked on hold may be implemented upon request.
    #    To get everything we want, we aptly build a set set of queries that
    # searches all categories. We return a set of pairs holding the query dictionary,
    # and the path to the index.php, absolute to domain root.
    #    Unfortunately, Library Genesis only allows to search one field at a time,
    # so when the wanted item specifies title AND authors AND publisher etc.,
    # we must do a query per field. Luckily, bookwyrm does the heavy lifting for us,
    # and some fields are unecessary, e.g. extension and year; these are not fuzzily matched.
    # TODO: investigate possibility of dupes. Hashmap?

    queries = []
    e = item.exacts
    ne = item.nonexacts

    #
    # The Text Book Category
    #

    # All appended queries below inherit the following keys
    base = {
        'req': 100,  # 100 results per page
        'view': 'simple'
    }

    search_for_in = lambda req, col: queries.append(
        ('/search.php', { **base, 'req': req, 'column': col})
    )

    if ne.title:
        search_for_in(ne.title, 'title')

    if ne.authors:
        search_for_in(', '.join(ne.authors), 'author')

    if ne.series:
        search_for_in(ne.series, 'series')

    if ne.publisher:
        search_for_in(ne.publisher, 'publisher')

    #
    # The Fiction Category
    #

    base = {
        'f_ext': e.extension or "All",
        'f_group': 0,  # Don't group results of differing extensions.
        'f_lang': 0,   # Search for all languages, for now.
    }

    fields = {'all': 0, 'title': 1, 'author': 2, 'series': 3}
    search_for_in = lambda s, col: queries.append(
        ('/foreignfiction/index.php', { **base, 's': s, 'f_column': col })
    )

    if ne.title:
        search_for_in(ne.title, fields['title'])

    if ne.authors:
        search_for_in(', '.join(ne.authors), fields['author'])

    if ne.series:
        search_for_in(ne.series, fields['series'])

    #
    # The Scientific Articles Category
    #

    def non_empty(*xs):
        res = [x for x in xs if x != bw.empty]
        return res[0] if res else None

    queries.append(
        ('/scimag/index.php',
        {
            's': ne.title,
            'journalid': ne.journal,
            'v': non_empty(e.volume, e.year),  # TODO: e.volume should be a string?
            #'i': add ne.issue?
            'p': non_empty(e.pages),
            'redirect': 0  # Don't redirect to Sci-Hub on no results
        })
    )

    return queries


def tables_fetcher(f):
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
    # we'll have to hack some invariants together:
    #     - /search.php: check if the last request matches the previous one;
    #     - /foreignfiction/index.php: check if the table is empty;
    # When the respective invariant is True, we've gone through all pages.
    last_request = None

    p = 1
    query_params = f.args.copy()

    extract_table = {
        '/search.php': lambda soup: soup.find('table', {'class':'c', 'rules':'rows'}, recursive=False),
        '/foreignfiction/index.php': lambda soup: soup.find_all('table', {'rules':'rows'})[-1],
    }

    # The ad-hoc'ed invariants does NOT look good. How can we make this better?
    while True:
        f.set({'page':p}).add(query_params)

        r = requests.get(f.url)
        if r.status_code != requests.codes.ok:
            # Log failure to bookwyrm?
            # Or log after taking exception?
            r.raise_for_status()

        soup = BeautifulSoup(r.text, 'html.parser')
        table = extract_table[str(f.path)](soup)

        if f.path == '/search.php' and r.text == last_request:
            # We've gone through all pages.
            return
        elif f.path == '/foreignfiction/index.php' and table.text == '':
            # Same thing here
            return

        yield table

        p += 1
        last_request = r.text


def process_libgen(table):
    """
    Processes a table soup from LibGen and returns the items found within.
    """
    # TODO: is the same HTML given when using gen.lib.rus.ec host?
    items = []

    if table == None:
        raise "table is non"

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
        has_series = False if stei.a.has_attr('title') else True

        # The title, edition, and isbn section always contain an id-attribute with
        # an integer.
        tei = stei.find_next('a', id=re.compile('\d+$'))

        def extract_title():
            try:
                # The title is the first sibling of the first font tag, if any
                dd = deque(tei.font.previous_siblings, maxlen=1)
                return dd.pop()
            except AttributeError:
                # No edtion of isbn numbers; so only the title is given.
                return tei.text

        def extract_edition():
            # Always surrounded by brackets, so look for those.
            for font in tei.find_all('font', recursive=False):
                if font.text.startswith('[') and font.text.endswith(']'):
                    return font.text[1:-1]

        nonexacts = bw.nonexacts_t({
            'series': stei.a.text if has_series else '',
            'title': extract_title(),
            'publisher': publisher.text,
            'edition': extract_edition() or '',
            'language': language.text
        }, authors.text.split(', '))

        def try_toint(s):
            try:
                return int(s.text)
            except ValueError:
                return bw.empty

        exacts = bw.exacts_t({
            'year': try_toint(year),
            'pages': try_toint(pages),
            'size': translate_size(size) or bw.empty
        }, extension.text)

        def extract_isbns():
            valid_isbn = lambda isbn: isbnlib.is_isbn10(isbn) or isbnlib.is_isbn13(isbn)

            # Always comma-seperated, so split those and check all elements
            for font in tei.find_all('font', recursive=False):
                if font.text.startswith('[') and font.text.endswith(']'):
                    # We stumbled upon the edition, again.
                    continue
                return [isbn for isbn in font.text.split(', ') if valid_isbn(isbn)]

        def extract_mirrors():
            libgenio, libgenpw, bookfi, b-ok = mirrors
            # first two are A-okay.
            # Both Bookfi  and b-ok requires the referer to be the previous page.

        # TODO: these mirrors must be processed!
        misc = bw.misc_t([mirror.a['href'] for mirror in mirrors], extract_isbns() or [])
        return (nonexacts, exacts, misc)

    # The first row is the column headers, so we skip it.
    for row in table.find_all('tr')[1:]:
        items.append(make_item(row))

    return items


def process_ffiction(table):
    """
    Processes a table soup from LibGen and returns the items found within.
    """
    # NOTE: this process only works on libgen.io queries. gen.lib.rus.ec does not
    # yield the same HTML.
    # TODO: resolve this!

    items = []

    if table == None:
        raise "table is none"

    def make_item(row):
        columns = row.find_all('td')

        authors, series, title, language, mirrors = columns

        # TODO: fix this code for multiple authors.
        # ',' is used to seperate first and last name, but what
        # is used for author seperation?

        nonexacts = bw.nonexacts_t({
            'series': series.text,
            'title': title.text,
            'language': language.text,
        }, [authors.text])

        extract_extension = lambda s: s.split('(', 1)[0]

        exacts = bw.exacts_t({}, extract_extension(mirrors.text))

        def extract_mirrors():
            # Two mirrors are offered: one libgen.io and one libgen.pw.
            # Both mirrors lead to intermediate download page(s).
            # The download URl must be extracted from these pages.
            # Both download links contain the md5-hash of the item (presumebly).
            # Can the final URL be deduced from this hash? (fetching these pages
            # really slows things down).
            # NOTE: Handle this in back-end? No, an intermediate page may fail.

            # NOTE: this happens to be None at times. Why?
            io, pw = mirrors.div.find_all('a')
            io = "http://" + f.host + io['href']
            pw = pw['href']

            urls = []

            try:  # libgen.io
                # Final URL contains same md5-hash, but an additional key parameter is
                # required (16 chars, alphanumeric, uppercase). Seems to be generated on
                # the fly. Or can it be solved for somehow?
                r = requests.get(io)
                soup = BeautifulSoup(r.text, 'html.parser')
                final = soup.table.find_all('td')[-1].a['href']
                urls.append(final)
            except:
                pass

            try:  # libgen.pw
                # Final URL contains another hash, which is always the same: the two hashes are
                # related. Now, is this a hash of the book itself, or the md5? (hash-finder hints
                # at CRC-96).
                r = requests.get(pw)
                soup = BeautifulSoup(r.text, 'html.parser')

                # Additionally, we can also get the file size from here.
                # But a side-effect would be real ugly. Is there a decent
                # way to extract this outside of "extract_mirrors"?
                #    Alternatively, the file size is also available in the main
                # view on the gen.lib.rus.ec domain.
                #params = soup.find('table', {'class':'book-item__params'})
                #entry = params.find_all('tr')[1].find_all('td')[-1]
                #size = translate_size(entry.text)

                # We can skip a third request by getting the libgen.pw's hash and
                # craft the final URL.
                hsh = soup.find('div', {'class':'book-info__download'}).a['href'].split('/')[-1]
                final = 'https://fiction.libgen.pw/download/book/' + hsh
                urls.append(final)
            except:
                pass

            return urls

        misc = bw.misc_t(extract_mirrors(), [])
        return (nonexacts, exacts, misc)


    for row in table.find_all('tr'):
        try:
            items.append(make_item(row))
        except (requests.exceptions.HTTPError, requests.exceptions.ConnectionError, AttributeError):
            continue

    return items


if __name__ == "__main__":
    nonexacts = bw.nonexacts_t({
        'title': 'Victory of Eagles',
        'series': 'Temeraire',
        },

        ['Naomi Novik']
    )

    item = bw.item((nonexacts, bw.exacts_t({},''), bw.misc_t([],[])))
    queries = build_queries(item)
    books = []
    domain = DOMAINS[0]

    for query in queries:
        for domain in DOMAINS:
            path, params = query
            f = furl('http://' + domain + path).set(query_params=params)
            print(f.url)

            if path != '/foreignfiction/index.php' and path != '/search.php':
                break

            try:
                for table in tables_fetcher(f):
                    print("next table")

                    if path == '/search.php':
                        books += process_libgen(table)
                    elif path == '/foreignfiction/index.php':
                        books += process_ffiction(table)
                    else:
                        print("unknown path")
            except requests.exceptions.ConnectionError:
                print("error: connection error")
                continue
            except requests.exceptions.HTTPError as e:
                print("http error:", e)
                continue

            # That domain worked; do the next query.
            break

    print('I found', len(books), 'books')
    for book in books:
        print(book)
