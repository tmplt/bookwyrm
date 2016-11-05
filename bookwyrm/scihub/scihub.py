"""
Fetch research papers from scihub.cc and it's mirrors.
"""

# Original author and source:
# <https://github.com/zaytoun/scihub.py/blob/master/scihub/scihub.py>

# This file has been edited in the following ways:
#   - rewritten for Python 3 and
#   - stripped of code not used by bookwyrm

import requests

MIRRORS = (
    'scihub.cc',
    'scihub.io'
)

# Can python requests's User-Agent be used instead?
HEADERS = {'User-Agent': 'Mozilla/5.0 (X11; Linux x86_64; rv:27.0) Gecko/20100101 Firefox/27.0'}


class SciHub(object):
    """
    Seach for research papers on Google Scholar and
    fetch papers from one of sci-hub's mirrors.
    """

    def __init__(self):
        pass

    def search(self, query, limit=10, download=False):
        """
        Performs a query on scholar.google.com, and returns a dictionary
        of results in the form {'papers': ...}. Unfortunately, as of now,
        captchas can potentially prevent searches after a certain limit.
        """
        start = 0
        results = {'papers': []}

        while True:
            error = None
            for mirror in MIRRORS:
                try:
                    base_url = "http://%s/" % mirror
                    r = requests.get(base_url, params={'q': query, 'start': start}, headers=HEADERS)
                except requests.exceptions.RequestException as e:
                    continue
                    # results['err'] = 'Failed to complete search with query %s (connection error)' % query
                    # return results

            soup = self._get_soup(r.content)
            papers = soup.find_all('div', class_="gs_r")

            if not papers:
                if 'CaptchaRedirect' in res.content:
                    results['err'] = 'Failed to complete search with query %s (captcha)' % query
                return results

            for paper in papers:
                if not paper.find('table'):
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

                    if len(results['papers']) >= limit:
                        return results

            start += 10

    def download(self, identifier, destination='', path=None):
        """
        Downloads a paper from sci-hub given an indentifier (DOI, PMID, URL).
        Currently, this can potentially be blocked by a captcha if a certain
        limit has been reached.
        """
        data = self.fetch(identifier)

        if not 'err' in data:
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

    def _get_soup(self, html):
        """
        Return html soup.
        """
        return BeautifulSoup(html, 'html.parser')

    def _generate_name(self, res):
        """
        Generate unique filename for paper. Returns a name by calcuating 
        md5 hash of file contents, then appending the last 20 characters
        of the url which typically provides a good paper identifier.
        """
        name = res.url.split('/')[-1]
        pdf_hash = hashlib.md5(res.content).hexdigest()
        return '%s-%s' % (pdf_hash, name[-20:])
