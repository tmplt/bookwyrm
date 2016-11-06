# coding: utf-8
# This function if from scihub.py by @zaytoun.
# Its source file, and license, can be found at
# <https://github.com/zaytoun/scihub.py/tree/ab02ef1ba0516d191bfdd683ad19f3d5712a0fae>

"""
Query Google Scholars for links to research papers.
"""

import requests
from bs4 import BeautifulSoup as bs

SCHOLARS_BASE_URL = "https://scholar.google.com/scholar"


def search(query, limit=10, download=False):
    """
    Performs a query on scholar.google.com, and returns a dictionary
    of results in the form {'papers': ...}. Unfortunately, as of now,
    captchas can potentially prevent searches after a certain limit.

    Potential errors can be found in papers['error'].
    """
    page_id = 0
    results = {'papers': []}

    while len(results['papers']) < limit:
        try:
            r = requests.get(SCHOLARS_BASE_URL, params={'q': query, 'start': page_id})
        except requests.exceptions.RequestException as e:
            results['error'] = "query: '%s'; reason: '%s'" % (query, e)
            return results

        soup = bs(r.content, 'html.parser')
        papers = soup.find_all('div', class_="gs_r")

        if not papers:
            if b'captcha' in r.content:
                results['error'] = "query: '%s'; reason: captcha" % query
            return results

        for paper in papers:
            if not paper.find('table'):  # is this necessary?
                source = None
                pdf = paper.find('div', class_='gs_ggs gs_fl')
                link = paper.find('h3', class_='gs_rt')

                if pdf:
                    source = pdf.find('a')['href']
                elif link.find('a'):
                    source = link.find('a')['href']
                else:
                    continue

                results['papers'].append({
                    'name': link.text,
                    'url': source
                })

        page_id += 10

    return results
