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
    html_row: a soup, but that of a html table row from libgen.
"""

import requests
from bs4 import BeautifulSoup as bs
from enum import IntEnum

from item import Item

mirrors = (
    'libgen.io',
    'gen.lib.rus.ec'
)

class column(IntEnum):
    id = 0
    author = 1
    title = 2
    isbns = 2
    publisher = 3
    year = 4
    # skip page count column,
    lang = 6
    # and file size.
    ext = 8
    mirrors = 9

class _fetcher(object):
    results = []

    # Note that any dictionary keys whose value is None
    # will not be added to the URL's query string.
    def __init__(self, query):
        r = None
        url = "http://%s/search.php"

        for mirror in mirrors:
            r = requests.get(url % mirror, params=query)
            if r.status_code == requests.codes.ok:
                # Don't bother with the other mirrors if
                # we already have what we wanted.
                break

        # If not 200, raises requests.exceptions.HTTPError
        # (or somethig else?)
        # TODO: catch this!
        r.raise_for_status()

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
        result = [r for n, r in enumerate(self.results) if n == self.current]
        if not result:
            raise StopIteration
        self.current += 1
        return result[0]

def _get_column(row, column):
    return row.find_all('td')[column]

def _get_author(row):
    soup = _get_column(row, column.author)

    author = soup.text.strip()
    return author if author else None

def _get_isbn(row):
    soup = _get_column(row, column.isbns)
    soup = soup.find('br').find('i')

    isbn = soup.text.split(', ')
    return isbn if isbn else None

def _get_title(row):
    soup = _get_column(row, column.title)

    def inner_isdigit(value):
        try:
            return value.isdigit()
        except AttributeError:
            return False

    # A book which is part of a serie will have the same row index
    # for the title as a book which isn't part of a series.
    # What the two books share, however, is that the <a> tag has the
    # book's ID (an integer) in the "id" attribute.
    # NOTE: can this be simplified by using BeautifulSoup's attributes?
    soup = soup.find(id=inner_isdigit)

    # This will affect the top-most object,
    # and since this strips ISBNs, we should probably extract those,
    # before running this. Course, we could probably deepcopy it,
    # but may not be necessary.
    rmtags = ['br', 'font']
    for tag in soup(rmtags):
        tag.decompose()

    title = soup.text.strip()
    return title if title else None

def _get_publisher(row):
    soup = _get_column(row, column.publisher)

    publisher = soup.text.strip()
    return publisher if publisher else None

def _get_year(row):
    soup = _get_column(row, column.year)

    year = soup.text.strip()
    return year if year else None

def _get_lang(row):
    soup = _get_column(row, column.lang)

    lang = soup.text.strip()
    return lang if lang else None

def _get_ext(row):
    soup = _get_column(row, column.ext)

    ext = soup.text.strip()
    return ext if ext else None

def get_results(query):
    # debugging and testing
    query = {'req': query.title}
    results = _fetcher(query)

    items = []
    for result in results:
        item = Item

        item.isbn = _get_isbn(result)
        item.author = _get_author(result)
        item.title = _get_title(result)
        item.publisher = _get_publisher(result)
        item.year = _get_year(result)
        item.lang = _get_lang(result)
        # item.doi = _get_doi(result)
        item.ext = _get_ext(result)

        items.append(item)

    return items

