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
from enum import Enum

from item import Item
import libgen

# Allow these to be set when initializing?
# Read from config/args for that?
class Sources(Enum):
    libgen = 1

    # Not yet implemented sources:
    # sci-hub = 2
    # irc = 3
    # torrents = 4

def search(item, source):

    def filter_unwanted(wanted, lst):
        for item in lst[:]:
            if item != wanted:
                lst.remove(item)

    results = []
    if source == Sources.libgen:
        results += libgen.search(item)

    filter_unwanted(item, results)

    return results

def print_items(items):
    for item in items:
        output = (
            "title:     %s" % item.title,
            "author:    %s" % ', '.join(item.authors),
            "publisher: %s" % item.publisher,
            "year:      %s" % item.year,
            "language:  %s" % item.lang,
            "edition:   %s" % item.edition,
            "extension: %s" % item.ext,
        )

        for line in output:
            print(line)
        print("--------------------")

def main(argv):
    parser = argparse.ArgumentParser(
            allow_abbrev=False,
            description='%(prog)s - find books and papers online and download them. \
                         When called with no arguments, bookwyrm prints this screen and exits.')

    addarg = parser.add_argument

    # Program functionality arguments.
    addarg('-a', '--author')
    addarg('-t', '--title')
    addarg('-p', '--publisher')
    addarg('-y', '--year', type=int)
    addarg('-l', '--language',
            help='Two letters denoting the item\'s language. e.g. \'en\' for English or \'sv\' for Swedish')
    addarg('-e', '--edition', type=int)
    addarg('-E', '--extension',
            help='Filename extension without period')
    addarg('-i', '--isbn')
    addarg('-d', '--doi')
    addarg('-u', '--url')

    # Utility.
    addarg('-v', '--verbose', action='count',
            help='Set verbosity level; more \'v\'s increases the level.')
    addarg('--version', action='version', version='%(prog)s no.ver.yet')

    args = parser.parse_args()

    if len(argv) < 2:
        parser.print_help()
        sys.exit(0)

    wanted = Item(args)

    found = []
    for source in Sources:
        found += search(wanted, source)

    cnt = len(found)
    if cnt > 0:
        print("I found %d items:" % cnt)
    else:
        print("I couldn't find anything.")
        sys.exit(1)

    print_items(found)


if __name__ == '__main__':
    sys.exit(main(sys.argv))

