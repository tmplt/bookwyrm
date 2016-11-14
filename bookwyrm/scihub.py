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
Fetch research papers from sci-hub.cc and it's mirrors.
"""

# Original author and source:
# <https://github.com/zaytoun/scihub.py/blob/master/scihub/scihub.py>

# This file has been edited in the following ways:
#   - rewritten for Python 3 and
#   - stripped of code not used by bookwyrm

from bs4 import BeautifulSoup as bs
from furl import furl
import requests
import hashlib
import logging

MIRRORS = (
    'sci-hub.cc',
    'sci-hub.io'
)


class SciHub:

    def __init__(self):
        self.logger = logging.getLogger('bookwyrm.scihub')

    def search_direct_url(self, ident):
        r = None

        for mirror in MIRRORS:
            url = "http://{}/{}".format(mirror, ident)
            self.logger.debug('trying url: \'%s\'' % url)

            r = requests.get(url)
            if r.status_code == requests.codes.ok:
                break

        r.raise_for_status()

        soup = bs(r.content, 'html.parser')

        try:
            return soup.iframe.get('src')
        except AttributeError:
            self.logger.debug('unable to find iframe; captcha?')
            return None

    def generate_name(self, r):
        """
        Generate unique filename for paper. Returns a name by calcuating
        sha1 hash of file contents, then appending the last 20 characters
        of the url which typically provides a good paper identifier.
        """

        f = furl(r.url)
        name = f.path.segments[-1]
        pdf_hash = hashlib.sha1(r.content).hexdigest()

        gen_name = "{}-{}".format(pdf_hash, name[-20:])

        self.logger.debug('generated filename: \'%s\'' % gen_name)
        return gen_name
