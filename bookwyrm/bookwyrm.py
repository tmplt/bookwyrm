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

class bookwyrm:

    def __init__(self, arg):
        self.arg = arg
        self.wanted = Item(arg)

        self.count = 0
        self.results = []

    def print_items(self):
        for idx, item in enumerate(self.results):
            title = item.data.title
            edition = item.exacts.edition
            ext = item.exacts.ext

            output = "%d | %s, %s, %s" % (
                idx,
                title,
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

        self.count = len(results) # used elsewhere
        return self.count

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
                # While the .php-script found in the 'action' attribute seems to be
                # "noleech1.php" for all items, the name seems temporary, so we extract that too.
                #
                # The <form> looks like the following:
                # <form name="receive" method="GET" onSubmit="this.submit.disabled=true;search.push.disabled=true;" action="noleech1.php">
                #     <input  name='hidden'  type='hidden'  value=item-UID-here>
                #     <input  name="hidden0" type="hidden"  value="item file name here">
                # </form>

                action_tag = soup.find('form', attrs={'name': 'receive'})
                action = action_tag['action']

                inputs = soup.find('input', attrs={'name': 'hidden'})
                uid = inputs['value'] # 'value' of the first <input>

                child = inputs.input
                filename = child['value']

                params = {
                    'hidden': uid,
                    'hidden0': filename
                }
                params = urlencode(params)

                # NOTE: HTTP refer(r)er "http://golibgen.io/" required to GET this.
                url = ('http://golibgen.io/%s?' % action) + params
                mirrors.append(url)

                continue

            if "bookzz" in url:
                # Every item is held within in a <div class="actionsHolder">,
                # but since this search is for an exact match, we will only ever
                # get one result.
                #
                # The <div> looks like the following (with useless data removed):
                # <div class="actionsHolder">
                #     <div style="float:left;">
                #         <a class="ddownload color2 dnthandler" href="http://bookzz.org/dl/1014779/9a9ab2" />
                #     </div>
                # </div>

                div = soup.find(attrs={'class': 'actionsHolder'})

                # NOTE: HTTP refer(r)er "http://bookzz.org/" required to GET this.
                url = div.div.a['href']
                mirrors.append(url)

                continue

        elif url.startswith("/ads"):
            # The relative link contains but one parameter which happens to
            # be the md5 of the item. To retrieve the .torrent-file, we only need
            # this.
            o = urlparse(url)
            md5_attr = o.query

            torrent_url = "http://libgen.io/book/index.php?%s&oftorrent=" % md5_attr
            r = requests.get(torrent_url)

            try:
                magnet = utils.magnet_from_torrent(r.content)
                mirrors.append(magnet)
            except bencodepy.DecodingError:
                pass

            continue

    return mirrors

def main(argv):
    parser = argparse.ArgumentParser(
            allow_abbrev=False,
            description='%(prog)s - find books and papers online and download them. \
                         When called with no arguments, bookwyrm prints this screen and exits.')

    addarg = parser.add_argument

    # Program functionality arguments.
    addarg('-a', '--author', action='append')
    addarg('-t', '--title')
    addarg('-s', '--serie')
    addarg('-p', '--publisher')
    addarg('-y', '--year', type=int)
    addarg('-l', '--language',
            #help='Two letters denoting the item\'s language. e.g. \'en\' for English or \'sv\' for Swedish')
            )
    addarg('-e', '--edition', type=int)
    addarg('-E', '--extension',
            help='filename extension without period, e.g. \'pdf\'.')
    addarg('-i', '--isbn')
    addarg('-d', '--doi')
    addarg('-u', '--url')

    # Utility.
    addarg('-v', '--verbose', action='count',
            help='verbose mode; prints out a lot of debug information. \
                  Can be used more than once, e.g. -vv, to increase the level of verbosity.')
    addarg('--version', action='version', version='%(prog)s 0.1.0-alpha.2')

    args = parser.parse_args()

    if len(argv) < 2: # at least one flag with an argument
        parser.print_help()
        return

    bw = bookwyrm(args)
    for source in Sources:
        found = bw.search(source)

    if found > 0:
        print("I found %d items!" % found)
    else:
        print("I couldn't find anything.")
        return Errno.no_results_found

    bw.print_items()

if __name__ == '__main__':
    sys.exit(main(sys.argv))

