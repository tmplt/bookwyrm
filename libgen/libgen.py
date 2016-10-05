# coding: utf-8

"""
This file contains functions for parsing search results of
Library Genesis and its mirrors.
    Fortunately, libgen gives us a html table, so we fetch each row,
zoom into the right column (with the help of _get_column and the column enums)
and extract the data we want in their respective _get_* function.

Common variable descriptions:
    soup: a BeautifulSoup.Resultset which we are gonna parse.
    html_row: a soup, but of a html table row from libgen.
"""

import requests
from bs4 import BeautifulSoup as bs
from enum import IntEnum
import item

mirrors = (
    'libgen.io',
    'gen.lib.rus.ec'
)

class column(IntEnum):
    id = 0
    author = 1
    title = 2
    publisher = 3
    year = 4
    # skip page count column
    lang = 6
    ext = 8

# Note that any dictionery key whose value if None
# will not be added to the URL's query string.
def _fetch_table(query):
    """Returns a list of search results."""

    r = None
    url = "http://%s/search.php"

    for mirror in mirrors:
        r = requests.get(url % mirror, params=query)
        if r.status_code == requests.codes.ok:
            # Don't bother with the other mirrors if we
            # already got what we wanted.
            break

    # If not 200, raises requests.exceptions.HTTPError
    # or something else.
    # TODO: catch this!
    r.raise_for_status()

    soup = bs(r.text, 'html.parser')

    results = []
    for row in soup.find_all('tr'):
        # The search result table gives every book an integer ID,
        # so we only want those table rows.
        if row.find('td').text.isdigit():
            #result = item(row)
            results.append(row)

#       for row in soup.find_all('tr'):
#           item = item(row)
#
#           for item in results:
#               if item.name = 

    return results

def _get_author(html_row):
    author = _get_column(html_row, column.author)

    if (author):
        return author.text.strip()
    return None

def _get_title(html_row):
    soup = _get_column(html_row, column.title)

    def inner_isdigit(value):
        try:
            return value.isdigit()
        except AttributeError:
            return False

    # A book which is part of a serie will have the same row index
    # for the title as a book which isn't part of a series.
    # What the two books share, however, is that the <a> tag has the
    # book's ID (an integer) in the "id" attribute.
    # TODO: can this be simplified by using BeautifulSoup's attributes?
    soup = soup.find(id=inner_isdigit)

    # This also affects the top-most project,
    # and since this strips ISBNs, we should probably extract those,
    # before running this.
    [x.decompose() for x in soup(['br', 'font'])]

    title = soup.text.strip()
    return title if title else None

def _get_publisher(html_row):
    pass

def _get_column(html_row, column):
    return html_row.find_all('td')[column]

def search(query):
    # debugging and testing
    query = {'req': query.title}
    found = _fetch_table(query)
    print("I found %d result(s)!" % len(found))
    print("The author is: %s" % _get_author(found[0]))
    print("The title is: %s" % _get_title(found[0]))

