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

from bs4 import BeautifulSoup
from furl import furl
from collections import deque
from enum import Enum
import requests
import re
import isbnlib
import tempfile

DOMAINS = ('libgen.io',)

class SoupError(Exception):
    def __init__(self, soup, error):
        self.soup = soup
        super().__init__('failed to parse soup: ' + str(error))

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
        'M': 1e6,
        'G': 1e9
    }

    # While LibGen lists sizes in '[kM]b', it's actually in bytes (B)
    return int(count * si_prefix.get(unit[0]))


class LibgenSeeker(object):
    def __init__(self, wanted, bookwyrm=FakeBookwyrm()):
        self.queries = self.build_queries(wanted)
        self.bookwyrm = bookwyrm

    def log(self, level, msg):
        if self.bookwyrm:
            self.bookwyrm.log(level, 'from ' + __file__ + ': ' + msg)
        else:
            print(msg)

    def feed(self, item):
        """
        Feed an item to the bookwyrm instance, otherwise print it out.
        """
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
        #   * Scientific Articles (/scimag/index.php);
        #   * Fiction (/foreignfiction/index.php);
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
        # The Text Book Category
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
        # The Fiction Category
        #

        base = {
            'f_ext': item['extension'] if 'extension' in item else "All",
            'f_group': 0,  # Don't group results of differing extensions.
            'f_lang': 0,   # Search for all languages, for now.
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
                            self.bookwyrm.log.war('unknown path "%s"; ignored.' % path)
                except requests.exceptions.ConnectionError as e:
                    self.bookwyrm.log.error('connection error (%s)!' % e)
                    continue
                except requests.exceptions.HTTPError as e:
                    self.bookwyrm.log.error('HTTP error (%s)!' % e)
                    continue
                except SoupError as e:
                    temp_file = tempfile.mktemp()
                    with open(temp_file, 'w') as fd:
                        fd.write(e.soup.prettify())

                    self.bookwyrm.log.error('unable to parse "%s"; somewhere a None appeared. Please submit a bug at ' % f.url +
                             '<https://github.com/Tmplt/bookwyrm/issues/new> and attach `%s`. Continuing...' % temp_file)
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
        # we'll have to hack some invariants together:
        #     - /search.php: check if the last request matches the previous one;
        #     - /foreignfiction/index.php: check if the table is empty;
        # When the respective invariant is True, we've gone through all pages.
        last_request = None

        p = 1
        query_params = f.args.copy()

        extract_table = {
            '/search.php': lambda soup: soup.find('table', {'class': 'c', 'rules': 'rows'}, recursive=False),
            '/foreignfiction/index.php': lambda soup: soup.find_all('table', {'rules': 'rows'})[-1],
        }

        # The ad-hoc'ed invariants does NOT look good. How can we make this better?
        while True:
            f.set({'page': p}).add(query_params)

            r = requests.get(f.url)
            if r.status_code != requests.codes.ok:
                # TODO: Log failure to bookwyrm?
                # Or log after taking exception?
                r.raise_for_status()

            soup = BeautifulSoup(r.text, 'html.parser')
            try:
                table = extract_table[str(f.path)](soup)
            except KeyError:
                self.bookwyrm.log.warn('cannot extract from "%s"; ignoring...' % f.path)
                raise NotImplementedError("only parsing for LibGen and ffiction currently supported.")
            except IndexError:
                self.bookwyrm.log.warn('unable to extract table from "%s"; ignoring...' % f.url)
                continue

            # Have we gone through all pages?
            # XXX: This really doesn't feel stable.
            if f.path == '/search.php' and r.text == last_request:
                return
            elif f.path == '/foreignfiction/index.php' and table.text == '':
                return

            yield table

            p += 1
            last_request = r.text

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

            nonexacts = {
                'series': stei.a.text if has_series else '',
                'title': extract_title(),
                'publisher': publisher.text,
                'edition': extract_edition() or '',
                'language': language.text,
                'authors': authors.text.split(', ')
            }

            def try_toint(s):
                try:
                    return int(s.text)
                except ValueError:
                    return bw.empty

            exacts = {
                'year': try_toint(year),
                'pages': try_toint(pages),
                'size': translate_size(size) or bw.empty,
                'extension': extension.text
            }

            def extract_isbns():
                def valid_isbn(isbn):
                    return isbnlib.is_isbn10(isbn) or isbnlib.is_isbn13(isbn)

                # Always comma-seperated, so split those and check all elements
                for font in tei.find_all('font', recursive=False):
                    if font.text.startswith('[') and font.text.endswith(']'):
                        # We stumbled upon the edition, again.
                        continue
                    return [isbn for isbn in font.text.split(', ') if valid_isbn(isbn)]

            def extract_mirrors():
                # libgenpw, libgenio, bookfi, bok = mirrors
                libgenpw = mirrors[0]
                libgenio = mirrors[1]
                # Only libgenpw can be downloaded from directly without fuss;
                # the rest require the intermediate page as HTTP referer.
                # TODO: process bookfi and B-Ok?

                libgenio = libgenio.a['href']
                libgenpw = libgenpw.a['href']

                urls = []

                # libgen.io
                #
                # Final URL contains same md5-hash, but an additional key parameter is
                # required (16 chars, alphanumeric, uppercase). Seems to be generated on
                # the fly. Or can it be solved for somehow?
                r = requests.get(libgenio)
                soup = BeautifulSoup(r.text, 'html.parser')
                # -2 here, but -1 on foreignfiction
                final = soup.table.find_all('td')[-2].a['href']
                urls.append(final)

                # libgen.pw
                #
                # Final URL contains another hash, which is always the same: the two hashes are
                # related. Now, is this a hash of the book itself, or the md5? (hash-finder hints
                # at CRC-96).
                r = requests.get(libgenpw)
                soup = BeautifulSoup(r.text, 'html.parser')

                # We can skip a third request by getting the libgen.pw's hash and
                # craft the final URL.
                hsh = soup.find('div', {'class': 'book-info__download'}).a['href'].split('/')[-1]
                # Yes, the exclusion of the subdomain matters! (fuck)
                final = 'https://libgen.pw/download/book/' + hsh
                urls.append(final)

                return urls

            misc = {
                # 'mirrors': extract_mirrors(),
                'isbns': extract_isbns() or []
            }

            return {**nonexacts, **exacts, **misc}

        # The first row is the column headers, so we skip it.
        # try:
            for row in table.find_all('tr')[1:]:
                # try:
                    self.bookwyrm.feed(make_item(row))
                # except AttributeError as e:
                    # raise SoupError(row, e)
        # except AttributeError as e:
            # raise SoupError(table, e)

    def process_ffiction(self, table):
        """
        Processes a table soup from LibGen and returns the items found within.
        """
        # NOTE: this process only works on libgen.io queries. gen.lib.rus.ec does not
        # yield the same HTML.
        # TODO: resolve this!

        def make_item(row):
            columns = row.find_all('td')

            authors, series, title, language, mirrors = columns

            # TODO: fix this code for multiple authors.
            # ',' is used to seperate first and last name, but what
            # is used for author seperation?

            nonexacts = {
                'series': series.text,
                'title': title.text,
                'language': language.text,
                'authors': [authors.text]
            }

            def extract_extension(s):
                return s.split('(', 1)[0]

            exacts = {'extension': extract_extension(mirrors.text)}

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
                io = "http://libgen.io" + io['href']
                pw = pw['href']

                urls = []

                # libgen.io
                #
                # Final URL contains same md5-hash, but an additional key parameter is
                # required (16 chars, alphanumeric, uppercase). Seems to be generated on
                # the fly. Or can it be solved for somehow?
                r = requests.get(io)
                soup = BeautifulSoup(r.text, 'html.parser')
                final = soup.table.find_all('td')[-1].a['href']
                urls.append(final)

                # libgen.pw
                #
                # Final URL contains another hash, which is always the same: the two hashes are
                # related. Now, is this a hash of the book itself, or the md5? (hash-finder hints
                # at CRC-96).
                r = requests.get(pw)
                soup = BeautifulSoup(r.text, 'html.parser')

                # We can skip a third request by getting the libgen.pw's hash and
                # craft the final URL.
                hsh = soup.find('div', {'class': 'book-info__download'}).a['href'].split('/')[-1]
                final = 'https://fiction.libgen.pw/download/book/' + hsh
                urls.append(final)

                return urls

            misc = {
                # 'mirrors': extract_mirrors()
            }

            return {**nonexacts, **exacts, **misc}

        for row in table.find_all('tr'):
            try:
                self.feed(make_item(row))
            except AttributeError as e:
                raise SoupError(row, e)


def find(wanted, bookwyrm):
    LibgenSeeker(wanted, bookwyrm).search()
    #bookwyrm.log.warn("title: %s" % wanted['title'])


if __name__ == "__main__":
    item = {
        'title': 'Victory of Eagles',
        'series': 'Temeraire',
        'authors': ['Naomi Novik']
    }

    LibgenSeeker(item).search()
