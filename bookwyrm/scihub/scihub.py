"""
Fetch research papers from scihub.cc and it's mirrors.
"""

# Original author and source:
# <https://github.com/zaytoun/scihub.py/blob/master/scihub/scihub.py>

# This file has been edited in the following ways:
#   - rewritten for Python 3 and
#   - stripped of code not used by bookwyrm

import os
import hashlib
import requests

SCIHUB_MIRRORS = (
    'scihub.cc',
    'scihub.io'
)

SCIHUB_BASE_URL = "http://scihub.cc/"

# Can python requests's User-Agent be used instead?
HEADERS = {'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64; rv:27.0) Gecko/20100101 Firefox/27.0'}


class SciHub(object):
    """
    Seach for research papers on Google Scholar and
    fetch papers from one of sci-hub's mirrors.
    """

    def __init__(self):
        pass

    def download(self, identifier, destination='', path=None):
        """
        Downloads a paper from sci-hub given an indentifier (DOI, PMID, URL).
        Currently, this can potentially be blocked by a captcha if a certain
        limit has been reached.
        """
        data = self.fetch(identifier)

        if 'err' not in data:
            self._save(data['pdf'],
                       os.path.join(destination, path if path else data['name']))

        return data

    def fetch(self, identifier):
        """
        Fetches the paper by first retrieving the direct link to the pdf.
        If the indentifier is a DOI, PMID, or URL pay-wall, then use Sci-Hub
        to access and download paper. Otherwise, just download paper directly.
        """
        url = self._get_direct_url(identifier)

        try:
            # verify=False is dangerous but sci-hub.io
            # requires intermediate certificates to verify
            # and requests doesn't know how to download them.
            # as a hacky fix, you can add them to your store
            # and verifying would work. will fix this later.
            res = requests.get(url, headers=HEADERS, verify=False)
            return {
                'pdf': res.content,
                'url': url,
                'name': self._generate_name(res)
            }
        except requests.exceptions.RequestException as e:
            return {
                'err': 'Failed to fetch pdf with identifier %s (resolved url %s) due to %s'
                % (identifier, url, 'failed connection' if url else 'captcha')
            }

    def _get_direct_url(self, identifier):
        """
        Finds the direct source url for a given identifier.
        """
        id_type = self._classify(identifier)

        return identifier if id_type == 'url-direct' \
            else self._search_direct_url(identifier)

    def _search_direct_url(self, identifier):
        """
        Sci-Hub embeds papers in an iframe. This function finds the actual
        source url which looks something like https://moscow.sci-hub.io/.../....pdf.
        """
        res = requests.get(SCIHUB_BASE_URL + identifier, headers=HEADERS, verify=False)
        s = self._get_soup(res.content)
        iframe = s.find('iframe')
        if iframe:
            return iframe.get('src')

    def _classify(self, identifier):
        """
        Classify the type of identifier:
        url-direct - openly accessible paper
        url-non-direct - pay-walled paper
        pmid - PubMed ID
        doi - digital object identifier
        """
        if (identifier.startswith('http') or identifier.startswith('https')):
            if identifier.endswith('pdf'):
                return 'url-direct'
            else:
                return 'url-non-direct'
        elif identifier.isdigit():
            return 'pmid'
        else:
            return 'doi'

    def _save(self, data, path):
        """
        Save a file give data and a path.
        """
        with open(path, 'wb') as f:
            f.write(data)

    def _generate_name(self, res):
        """
        Generate unique filename for paper. Returns a name by calcuating
        md5 hash of file contents, then appending the last 20 characters
        of the url which typically provides a good paper identifier.
        """
        name = res.url.split('/')[-1]
        pdf_hash = hashlib.md5(res.content).hexdigest()
        return '%s-%s' % (pdf_hash, name[-20:])
