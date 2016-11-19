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
This module will HTTP GET a result table from Library Genesis (_LibGen())
with the given query (search()), iterate through each table row and extract the
row data into an Item (_Row.as_item()).

A list of found Items is then returned.
"""

from bs4 import BeautifulSoup as bs
from enum import IntEnum
from furl import furl
from urllib.parse import urlencode, urlparse
import requests
import re
import bencodepy
import logging

from item import Item, NonExacts, Exacts, Misc
import utils

MIRRORS = (
    'libgen.io',
    'gen.lib.rus.ec'
)


class column(IntEnum):
    id = 0
    authors = 1
    title = 2
    serie = 2
    isbns = 2
    edition = 2
    publisher = 3
    year = 4
    pages = 5
    lang = 6
    size = 7
    ext = 8
    mirrors_start = 9
    mirrors_end = 12


class _Row(object):
    """
    Store and retrieve data from a html table row.
    """

    def __init__(self, row):
        self.row = row

    def _get_column(self, column):
        return self.row.find_all('td')[column]

    def _get_authors(self):
        soup = self._get_column(column.authors)

        delim = r'[,;]'  # NOTE: 'and' is also used, it seems
        authors = re.split(delim, soup.text.strip())

        # NOTE: can we strip with re.split() instead?
        authors = [author.strip() for author in authors]

        return authors if authors else None

    def _get_title(self):
        soup = self._get_column(column.title)

        def inner_isdigit(value):
            try:
                return value.isdigit()
            except AttributeError:
                return False

        # Book series name, its title, the edition of it and all of its
        # ISBN numbers share the same column. Luckily, the title's tag will
        # have an 'id'-attribute containing a digit, differing it from the
        # other tags in the column; we search for that tag.
        #
        # NOTE: the 'id'-attribute only appears where the title text is.
        #       Perhaps we don't need to verify it is a digit?
        soup = soup.find(id=inner_isdigit)

        # The found tags are references, so the top-most object will be affected,
        # and since this strips ISBNs, we will extract those before running this.
        rmtags = ['br', 'font']
        for tag in soup(rmtags):
            tag.decompose()

        title = soup.text.strip()
        return title if title else None

    def _get_serie(self):
        soup = self._get_column(column.serie)

        url = soup.a['href']
        f = furl(url)

        try:
            serie = f.args['req']
        except KeyError:
            serie = None

        return serie

    def _get_isbns(self):
        soup = self._get_column(column.isbns)
        try:
            soup = soup.find('br').find('i')

            isbns = soup.text.split(', ')
            return isbns if isbns else None
        except AttributeError:  # not all entries will have ISBN numbers
            return None

    def _get_edition(self):
        soup = self._get_column(column.edition)
        soups = soup.find_all('i')

        for soup in soups:
            try:
                edition = soup.text
            except AttributeError:
                return None

            # Item editions are always incased in brackets,
            # e.g. '[6ed.]', '[7th Revised Edition]'.
            if edition[0] != '[':
                continue

            # We could use substring to get the edition number,
            # but in the case that the number is more than one digit,
            # we regex it instead.
            try:
                edition = int(re.findall(r'\d+', edition)[0])
                return edition
            except (TypeError, ValueError, IndexError):
                return None

    def _get_publisher(self):
        soup = self._get_column(column.publisher)

        publisher = soup.text.strip()
        return publisher if publisher else None

    def _get_year(self):
        soup = self._get_column(column.year)

        try:
            year = int(soup.text.strip())
        except ValueError:
            return None
        return year

    def _get_page_count(self):
        soup = self._get_column(column.pages)

        count = soup.text.strip()
        return count if count else None

    def _get_lang(self):
        soup = self._get_column(column.lang)

        lang = soup.text.strip()
        return lang.lower() if lang else None

    def _get_size(self):
        """
        Return size in bytes;
        e.g., "816 kb" -> "816 * 10^3 / 8"
        """
        soup = self._get_column(column.size)

        size = soup.text.strip().split()
        number = size[0]
        prefix = size[0][1]  # without the 'b' in e.g. 'kb'

        bytesize = int(number * utils.translate_si_prefix(prefix) / 8)
        return bytesize

    def _get_ext(self):
        soup = self._get_column(column.ext)

        ext = soup.text.strip()
        return ext if ext else None

    def _get_mirrors(self):
        urls = []
        for col in range(column.mirrors_start, column.mirrors_end):
            soup = self._get_column(col)
            url = soup.find('a')['href']
            urls.append(url)

        return urls

    def as_item(self):
        # Existing in the same column as the title -- for which
        # we must decompose all <i>-tags -- these must be extracted first.
        edition = self._get_edition()
        isbns = self._get_isbns()

        nonexacts = NonExacts(
            authors = self._get_authors(),
            title = self._get_title(),
            serie = self._get_serie(),
            publisher = self._get_publisher()
        )

        exacts = Exacts(
            year = self._get_year(),
            lang = self._get_lang(),
            edition = edition,
            ext = self._get_ext(),
            pages = self._get_page_count()
        )

        misc = Misc(
            isbns = isbns,
            mirrors = self._get_mirrors()
        )

        return Item(
            nonexacts = nonexacts,
            exacts = exacts,
            misc = misc
        )


class _LibGen:
    """
    Retrieve the html table and iterate over its rows.
    """

    logger = None
    results = []

    def __init__(self, query, logger):
        self.logger = logger
        self._search(query)

    def _search(self, query):
        r = None
        filename = "/tmp/bookwyrm-%s" % query['req']

        try:
            with open(filename, 'r') as f:
                r = f.read()
                self.logger.debug('\'%s\' exists' % filename)

            soup = bs(r, 'html.parser')

        except IOError:
            url = "http://%s/search.php"

            for mirror in MIRRORS:
                self.logger.debug('fetching from %s...' % mirror)

                # Note that any dictionary keys whose value is None
                # will not be added to the URL's query string.
                r = requests.get(url % mirror, params=query)
                if r.status_code == requests.codes.ok:
                    # Don't bother with the other mirrors if
                    # we already have what we want.
                    self.logger.debug('...fetched!')
                    break
                else:
                    self.logger.debug('request returned code %d, throwing...' % r.status_code)

            # If not 200, raises requests.exceptions.HTTPError
            # (or somethig else?)
            # TODO: catch this!
            r.raise_for_status()

            with open(filename, 'w+') as f:
                f.write(r.text)

            soup = bs(r.text, 'html.parser')

        for row in soup.find_all('tr'):
            # The search result table gives every item an integer ID,
            # so we only want those rows.
            if row.find('td').text.isdigit():
                self.results.append(row)

    def __iter__(self):
        return iter(self.results)


def process_mirrors(urls, source=None):
    """
    Parse all mirrors and return URIs which can be downloaded
    with a single request.
    """

    mirrors = []
    for url in urls:
        if url.startswith('http'):
            r = requests.get(url)
            soup = bs(r.text, 'html.parser')

            if "golibgen" in url:
                # To retrieve an item from golibgen, we must construct a query
                # which contains the item's unique ID and it's file name.
                #
                # Golibgen gives us a <form> which contains all this data.
                # the uid can be found in the 'hidden' attribute and the file
                # name in 'hidden0'.
                #
                # While the .php-script found in the 'action' attribute seems
                # to be "noleech1.php" for all items, the name seems temporary,
                # so we extract that too.
                #
                # The <form> looks like the following:
                # <form name="receive" method="GET" action="noleech1.php">
                #   <input name="hidden"  type="hidden" value="item-UID">
                #   <input name="hidden0" type="hidden" value="filename">
                # </form>

                action_tag = soup.find('form', attrs={'name': 'receive'})
                action = action_tag['action']

                inputs = soup.find('input', attrs={'name': 'hidden'})
                uid = inputs['value']  # 'value' of the first <input>

                child = inputs.input
                filename = child['value']

                params = {
                    'hidden': uid,
                    'hidden0': filename
                }
                params = urlencode(params)

                # NOTE: HTTP refer(r)er "http://golibgen.io/" required
                # to GET this.
                url = ('http://golibgen.io/%s?' % action) + params
                mirrors.append(url)

                continue

            if "bookzz" in url:
                # Every item is held within in a <div class="actionsHolder">,
                # but since this search is for an exact match, we will only
                # ever get one result.
                #
                # The <div> looks like the following:
                # <div class="actionsHolder">
                #   <div style="float:left;">
                #    <a class="ddownload color2 dnthandler" href="..." />
                #   </div>
                # </div>

                div = soup.find(attrs={'class': 'actionsHolder'})

                # NOTE: HTTP refer(r)er "http://bookzz.org/" required
                # to GET this.
                url = div.div.a['href']
                mirrors.append(url)

                continue

        elif url.startswith("/ads"):
            # The relative link contains but one parameter which happens to
            # be the md5 of the item.
            # To retrieve the .torrent-file, we only need this.
            o = urlparse(url)
            md5 = o.query

            torrent_url = "http://libgen.io/book/index.php?%s&oftorrent=" % md5
            r = requests.get(torrent_url)

            try:
                magnet = utils.magnet_from_torrent(r.content)
                mirrors.append(magnet)
            except bencodepy.DecodingError:
                pass

            continue

    return mirrors


def search(query):
    """
    Search libgen for `query` and sort the results into a list of Items.
    """

    logger = logging.getLogger('bookwyrm.libgen')

    # debugging and testing
    query = {
        'req': query.nonexacts.title,
        'view': 'simple'
    }
    logger.debug('querying with \'%s\'' % query)

    items = []
    for result in _LibGen(query, logger):
        row = _Row(result)
        item = row.as_item()

        items.append(item)

    logger.debug('I found %d items traversing libgen' % len(items))
    return items
