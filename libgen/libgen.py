# coding: utf-8

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
    # skip page count
    lang = 6
    ext = 8

# Note that any dictionery key whose value if None
# will not be added to the URL's query string.

def _fetch_results(query):
    """Returns a list of search results."""

    r = None
    for mirror in mirrors:
        # "/search.php" isn't required here.
        r = requests.get('http://' + mirror, params=query)
        if r.status_code == requests.codes.ok:
            break

    # If not 200, raises requests.exceptions.HTTPError
    # or something else.
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
    author = _get_entity(html_row, column.author)

    if (author):
        return author.text.strip()
    return None

def _get_title(html_row):
    soup = _get_entity(html_row, column.title)

    def isdigit(value):
        try:
            return value.isdigit()
        except AttributeError:
            return False

    # A book which is part of a serie will have the same row index
    # for the title as a book which isn't part of a series.
    # What the two books share, however, is that the <a> tag has the
    # book's ID (an integer) in the "id" attribute.
    # TODO: can this be simplified by using BeautifulSoup's attributes?
    soup = soup.find(id=isdigit)
    [x.decompose() for x in soup(['br', 'font'])] # also affects object above html_row
    title = soup.text.strip()

    return title if title else None

def _get_publisher(html_row):
    pass

def _get_entity(html_row, column):
    return html_row.find_all('td')[column]

def search(query):
    query = {'req': query}
    found = _fetch_results(query)
    print("I found %d result(s)!" % len(found))

