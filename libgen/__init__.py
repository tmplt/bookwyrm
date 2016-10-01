# coding: utf-8

"""Fetch URIs from libgen.io and its mirrors"""

import requests
from bs4 import BeautifulSoup as bs

mirrors = (
    'libgen.io',
    'gen.lib.rus.ec'
)

# Note that any dictionery key whose value if None
# will not be added to the URL's query string.

def _fetch_results(query):
    """Returns a list of search results."""
    #query = {'req': 'temeraire'}
    query = {'req': 'we were soldiers once'}

    r = None
    for mirror in mirrors:
        r = requests.get('http://' + mirror, params=query)
        if r.status_code == requests.codes.ok:
            break

    # if not 200, raises requests.exceptions.HTTPError
    # or something else.
    r.raise_for_status()

    r = requests.get('http://' + mirrors[0], params=query)
    soup = bs(r.text, 'html.parser')

    results = []
    for row in soup.find_all('tr'):
        if row.find('td').text.isdigit():
            results.append(row)

    return results


def _format_results():
    # format the result of _fetch_results into a PoD
    # which we can iterate over?
    pass

def results():
    query = {'req': 'temeraire'}
    found = _fetch_results(query)
    print("I found %d result(s)!" % len(found))
