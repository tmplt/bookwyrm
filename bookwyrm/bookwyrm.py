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

from item import Item
import libgen

# Allow these to be set when initializing?
# Read from config/args for that?
sources = (
    'libgen',

# Not yet implemented sources:
#    'sci-hub',
#    'irc',
#    'torrents'
)

def search(source, query):
    if source == "libgen":
        return libgen.search(query)
    else:
        print("Unsupported source.")
        sys.exit(2)

def main(argv):
    parser = argparse.ArgumentParser(
            allow_abbrev=False,
            description='%(prog)s - find books and papers online and download them. \
                         When called with no arguments, bookwyrm prints this screen and exits.')

    addarg = parser.add_argument

    # Program functionality arguments.
    addarg('-a', '--author', required=False)
    addarg('-t', '--title', required=False)
    addarg('-p', '--publisher', required=False)
    addarg('-y', '--year', type=int, required=False)
    addarg('-l', '--language', required=False,
            help='Two letters denoting the item\'s language. e.g. \'en\' for English or \'sv\' for Swedish')
    addarg('-e', '--edition', type=int, required=False)
    addarg('-E', '--extension', required=False,
            help='Filename extension without period')
    addarg('-i', '--isbn', required=False)
    addarg('-d', '--doi', required=False)

    # Utility.
    addarg('-v', '--verbose', action='count',
            help='Set verbosity level; more \'v\'s increases the level.')
    addarg('--version', action='version', version='%(prog)s no.ver.yet')

    args = parser.parse_args()

    if len(argv) < 2:
        parser.print_help()
        sys.exit(0)

    wanted = Item(args)
    found = libgen.get_results(wanted)

    print("Found %d items!" % len(found))
    for item in found:
        output = (
            "title: %s" % item.title,
            "author: %s" % item.author,
            "publisher: %s" % item.publisher,
            "year: %s" % item.year,
            "language: %s" % item.lang,
            "edition: %s" % item.edition,
            "extension: %s" % item.ext,
            "mirror: %s" % str(item.mirrors)
        )

        for line in output:
            print(line)
        print("--------------------")

#    query = args.query
#
#    for source in sources:
#        r = search(source, query)

if __name__ == '__main__':
    sys.exit(main(sys.argv))

