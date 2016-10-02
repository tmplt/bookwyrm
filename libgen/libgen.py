# coding: utf-8

import requests
from bs4 import BeautifulSoup as bs
from .item import item

mirrors = (
    'libgen.io',
    'gen.lib.rus.ec'
)

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

    return results

def _format_results():
    # format the result of _fetch_results into a PoD
    # which we can iterate over?
    pass

def search(query):
    query = {'req': query}
    found = _fetch_results(query)
    print("I found %d result(s)!" % len(found))

