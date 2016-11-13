#!/usr/bin/env python
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

from __future__ import print_function

import sys
import argparse
import requests
import bencodepy
from enum import Enum, IntEnum
from bs4 import BeautifulSoup as bs
from urllib.parse import urlparse, urlencode

from item import Item
import libgen
import utils
import scihub


# Allow these to be set when initializing?
# Read from config/args for that?
class Sources(Enum):
    libgen = 1

    # Not yet implemented sources:
    # sci-hub = 2
    # irc = 3
    # torrents = 4


class Errno(IntEnum):
    no_results_found = 1


class IdentType(Enum):
    direct = 0
    paywall = 1
    doi = 2


class bookwyrm:

    def __init__(self, arg):
        self.arg = arg
        self.wanted = Item(arg)

        self.count = 0
        self.results = []

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        pass

    def print_items(self):
        for idx, item in enumerate(self.results):
            edition = item.exacts.edition
            ext = item.exacts.ext

            output = "%d | %s, %s, %s" % (
                idx,
                item.title,
                utils.ordinal_num(edition) + " ed." if edition else "n/a ed.",
                ext
            )

            print(output)

    def search(self, source):

        # NOTE: This is bad code:
        # - __eq__ for all items is run twice (.remove()).
        # - this could probably be made into a zero-parameter function.
        # - the bloody list is copied, which isn't very efficient.
        def filter_unwanted(wanted, lst):
            for item in lst[:]:
                if not item.matches(wanted):
                    lst.remove(item)

            return lst

        if source == Sources.libgen:
            results = libgen.search(self.wanted)

        self.results = filter_unwanted(self.wanted, results)

        self.count = len(results)  # used elsewhere
        return self.count

    def fetch(self, ident):
        url = self._get_direct_url(ident)

        try:
            r = requests.get(url)

            return {
                'pdf': r.content,
                'url': url,
                'name': scihub.generate_name(r)
            }
        except requests.exceptions.RequestException as e:
            print(e)

    def _get_direct_url(self, ident):
        id_type = self._classify(ident)

        if id_type == IdentType.direct:
            return ident
        else:
            return scihub.search_direct_url(ident)

    def _classify(self, ident):
        if ident.startswith('http'):
            if ident.endswith('pdf'):
                return IdentType.direct
            else:
                return IdentType.paywall
        elif utils.valid_doi(ident):
            return IdentType.doi
        else:
            return None


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


def parse_command_line(parser):
    # NOTE: Keep an eye on <https://bugs.python.org/issue11588>
    add_optarg = parser.add_argument
    addgroup = parser.add_argument_group

    # Main arguments; one of these are required.
    main = addgroup('necessarily inclusive arguments; at least one required')
    main.add_argument('-a', '--author', action='append')
    main.add_argument('-t', '--title')
    main.add_argument('-s', '--serie')
    main.add_argument('-p', '--publisher')

    # Exact data arguments; all optional
    exact = addgroup('exact arguments; optional')
    exact.add_argument('-y', '--year', type=int)
    exact.add_argument('-l', '--language')
    exact.add_argument('-e', '--edition', type=int)
    exact.add_argument('-E', '--extension',
                       help='filename extension without period, e.g. \'pdf\'.')
    exact.add_argument('-i', '--isbn')
    exact.add_argument('-d', '--ident')

    # Utility arguments; optional
    add_optarg('-v', '--verbose', action='count',
               help='''
               verbose mode; prints out a lot of debug information.
               Can be used more than once, e.g. -vv,
               to increase the level of verbosity.
               ''')
    add_optarg('--version', action='version', version='%(prog)s 0.1.0-alpha.2')
    add_optarg('--debug', action='store_true')

    args = parser.parse_args()

    return args


def main(argv):
    parser = argparse.ArgumentParser(
        prog='bookwyrm',
        allow_abbrev=False,
        description='''
        %(prog)s - find books and papers online and download them.
        When called with no arguments,
        bookwyrm prints this screen and exits.
        ''',
        add_help=True
    )

    args = parse_command_line(parser)

    if len(argv) < 2:  # no arguments given
        parser.print_help()
        return

    elif not (args.author or args.title or args.serie or args.publisher or args.ident):
        parser.error('At least a title, serie, publisher, author or ident must be specified.')

    with bookwyrm(args) as bw:
        if args.ident:
            pdf = bw.fetch(args.ident)
            utils.write(pdf)
            return

        for source in Sources:
            bw.search(source)

        if bw.count > 0:
            print("I found %d items!" % bw.count)
        else:
            print("I couldn't find anything.")
            return Errno.no_results_found

        bw.print_items()

if __name__ == '__main__':
    sys.exit(main(sys.argv))
