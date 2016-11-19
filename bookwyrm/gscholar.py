# coding: utf-8
# This function if from scihub.py by @zaytoun.
# Its source file, and license, can be found at
# <https://github.com/zaytoun/scihub.py/tree/ab02ef1ba0516d191bfdd683ad19f3d5712a0fae>

"""
Query Google Scholars for links to research papers.
"""

from bs4 import BeautifulSoup as bs
import requests
import logging
import bibtexparser

from item import Item, NonExacts, Exacts, Misc

SCHOLARS_BASE_URL = "https://scholar.google.com/scholar"


class _GScholar:

    logger = None
    results = []

    def __init__(self, query, logger):
        self.logger = logger
        self._search(query)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        pass

    def _get_soup(self, res):
        return bs(res.content, 'html.parser')

    def _search(self, query, limit=10):
        """
        Performs a query on scholar.google.com, and returns a dictionary
        of results in the form {'papers': ...}. Unfortunately, as of now,
        captchas can potentially prevent searches after a certain limit.

        Potential errors can be found in papers['error'].
        """

        page_id = 0

        while len(self.results) < limit:
            try:
                self.logger.debug('getting result page')
                res_page = requests.get(SCHOLARS_BASE_URL, params={'q': query, 'start': page_id})
            except requests.exceptions.RequestException as e:
                self.logger.warning('query: \'{}\'; reason: \'{}\''.format(query, e))

            soup = self._get_soup(res_page)
            papers = soup.find_all('div', class_="gs_r")

            if len(papers) == 0:
                if b'captcha' in res_page.content:
                    raise KeyError('captcha')  # find something more apropriate
                return

            self.logger.debug('found %d paper(s)' % len(papers))

            for paper in papers:
                link = paper.find('h3', class_='gs_rt')

                try:
                    url = link.a.get('href')
                except AttributeError:
                    self.logger.debug('unable to parse paper (%s)' % link)
                    continue

                if url:
                    paper_combo = (paper, url)
                    self.results.append(paper_combo)

            if len(papers) < 10:
                # Don't fetch the next page if the last results
                # are on the current page.
                break

            page_id += 10

    def _get_bibtex(self, paper):
        javascript = paper.find('a', class_='gs_nph', role='button').get('onclick')
        uid = extract_uid(javascript)
        self.logger.debug('uid %s extracted; fetching cite page' % uid)

        params = {
            'q': 'info:{}:scholar.google.com'.format(uid),
            'output': 'cite'
        }
        cite_page = requests.get(SCHOLARS_BASE_URL + '.bib', params=params)
        soup = self._get_soup(cite_page)

        bib_url = soup.find('a', class_='gs_citi').get('href')
        self.logger.debug('cite page fetched; fetching bibtex file')
        bib = requests.get(bib_url)

        return bib.text

    def as_item(self, result):
        paper, download_url = result

        bibtex_file = self._get_bibtex(paper)
        bib = bibtexparser.loads(bibtex_file).entries[0]

        nonexacts = NonExacts(
            authors = bib.get('author', None),
            title = bib.get('title', None),
            publisher = bib.get('publisher', bib.get('organization', None)),
            journal = bib.get('journal', None)
        )

        exacts = Exacts(
            year = bib.get('year', None),
            volume = bib.get('volume', None),
            number = bib.get('number', None),
            pages = bib.get('pages', None)
        )

        misc = Misc(
            mirrors = [download_url]
        )

        return Item(
            nonexacts = nonexacts,
            exacts = exacts,
            misc = misc
        )

    def __iter__(self):
        return iter(self.results)


def extract_uid(js_string):
    """
    Extracts the second javascript function argument,
    the unique id, from strings similar to
    "return gs_ocit(event, 'DJXe6QLE6UIJ', '0')"
    """

    s = js_string
    args = s[s.find('(') + 1:s.find(')')]
    args = args.replace('\'', '').split(',')

    return args[1]


def search(item):
    """
    Search Google Scholar for `query` and sort the results into a list of Items.
    """

    logger = logging.getLogger('bookwyrm.gscholar')

    query = {'req': item.nonexacts.title}
    logger.debug('querying with \'%s\'' % query)

    results = []
    with _GScholar(query, logger) as gs:
        for result in gs:
            paper = gs.as_item(result)
            results.append(paper)

    return results
