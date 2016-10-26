# coding: utf-8
# This file is part of bookwyrm.
# Copyright 2016, Tmplt.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.

"""
This file contains functions for parsing search results of
Library Genesis and its mirrors.
    Fortunately, libgen gives us a html table, so we fetch each row,
zoom into the right column (with the help of _get_column and the column enums)
and extract the data we want in their respective _get_* function.

Common variable descriptions:
    soup: a BeautifulSoup.Resultset which we are gonna parse.
"""

from bs4 import BeautifulSoup as bs
from enum import IntEnum
import requests
import re

from item import Item

MIRRORS = (
    'libgen.io',
    'gen.lib.rus.ec'
)

class column(IntEnum):
    id = 0
    authors = 1
    title = 2
    isbns = 2
    edition = 2
    publisher = 3
    year = 4
    # skip page count column,
    lang = 6
    # and file size.
    ext = 8
    mirrors_start = 9
    mirrors_end = 12

class _fetcher(object):
    results = []

    # Note that any dictionary keys whose value is None
    # will not be added to the URL's query string.
    def __init__(self, query):
        r = None
        filename = "/tmp/bookwyrm-%s" % query['req']

        try:
            with open(filename, 'r') as f:
                r = f.read()
            soup = bs(r, 'html.parser')

        except IOError:
            url = "http://%s/search.php"

            for mirror in MIRRORS:
                r = requests.get(url % mirror, params=query)
                if r.status_code == requests.codes.ok:
                    # Don't bother with the other mirrors if
                    # we already have what we want.
                    print("fetched from libgen!")
                    break

            # If not 200, raises requests.exceptions.HTTPError
            # (or somethig else?)
            # TODO: catch this!
            r.raise_for_status()

            with open(filename, 'w+') as f:
                f.write(r.text)

            soup = bs(r.text, 'html.parser')

        for row in soup.find_all('tr'):
            # The search result table gives every item an integer ID,
            # so we only want those rows.
            if row.find('td').text.isdigit():
                self.results.append(row)

    def __iter__(self):
        self.current = 0
        return self

    def __next__(self):
        # NOTE: superfluous code, since self.results is a list already.
        result = [r for n, r in enumerate(self.results) if n == self.current]
        if not result:
            raise StopIteration
        self.current += 1
        return result[0]

def _get_column(row, column):
    return row.find_all('td')[column]

def _get_authors(row):
    soup = _get_column(row, column.authors)

    delim = r'[,;]' # NOTE: 'and' is also used, it seems
    authors = re.split(delim, soup.text.strip())

    # NOTE: can we strip with re.split() instead?
    authors = [author.strip() for author in authors]

    return authors if authors else None

def _get_title(row):
    soup = _get_column(row, column.title)

    def inner_isdigit(value):
        try:
            return value.isdigit()
        except AttributeError:
            return False

    # Book series name, its title, the edition of it and all of its
    # ISBN numbers share the same column. Luckily, the title's tag will
    # have an 'id'-attribute containing a digit, differing it from the
    # other tags in the column; we search for that tag.
    #
    # NOTE: the 'id'-attribute only appears where the title text is.
    #       Perhaps we don't need to verify it is a digit?
    soup = soup.find(id=inner_isdigit)

    # The found tags are references, so the top-most object will be affected,
    # and since this strips ISBNs, we will extract those before running this.
    rmtags = ['br', 'font']
    for tag in soup(rmtags):
        tag.decompose()

    title = soup.text.strip()
    return title if title else None

def _get_isbns(row):
    soup = _get_column(row, column.isbns)
    try:
        soup = soup.find('br').find('i')

        isbns = soup.text.split(', ')
        return isbns if isbns else None
    except AttributeError: # not all entries will have ISBN numbers
        return None

def _get_edition(row):
    soup = _get_column(row, column.edition)
    soups = soup.find_all('i')

    for soup in soups:
        try:
            edition = soup.text
        except AttributeError:
            return None

        # Item editions are always incased in brackets,
        # e.g. '[6ed.]', '[7th Revised Edition]'.
        if edition[0] != '[':
            continue

        # We could use substring to get the edition number,
        # but in the case that the number is more than one digit,
        # we regex it instead.
        try:
            edition = int(re.findall(r'\d+', edition)[0])
            return edition
        except (TypeError, ValueError, IndexError):
            return None

def _get_publisher(row):
    soup = _get_column(row, column.publisher)

    publisher = soup.text.strip()
    return publisher if publisher else None

def _get_year(row):
    soup = _get_column(row, column.year)

    try:
        year = int(soup.text.strip())
    except ValueError:
        return None
    return year

def _get_lang(row):
    soup = _get_column(row, column.lang)

    lang = soup.text.strip()
    return lang.lower() if lang else None

def _get_ext(row):
    soup = _get_column(row, column.ext)

    ext = soup.text.strip()
    return ext if ext else None

def _get_mirrors(row):
    urls = []
    for col in range(column.mirrors_start, column.mirrors_end):
        soup = _get_column(row, col)
        url = soup.find('a')['href']
        urls.append(url)

    return urls


def search(query):
    # debugging and testing
    query = {
        'req': query.title,
        'view': 'simple'
    }

    results = _fetcher(query)

    items = []
    for result in results:
        item = Item()

        # Existing in the same column as the title -- for which
        # we must decompose all <i>-tags -- these must be extracted first.
        item.edition = _get_edition(result)
        item.isbns = _get_isbns(result)

        item.authors = _get_authors(result)
        item.title = _get_title(result)
        item.publisher = _get_publisher(result)
        item.year = _get_year(result)
        item.lang = _get_lang(result)
        item.ext = _get_ext(result)
        item.mirrors = _get_mirrors(result)

        items.append(item)

    return items

