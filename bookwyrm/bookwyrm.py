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
                         When called with no arguments, bookwyrm prints this screen and exits.',
            epilog='Licensed under the MIT license. Created by Tmplt <ttemplate223@gmail.com>. \
                    Please report any bugs to my email or at https://github.com/Tmplt/bookwyrm')

    addarg = parser.add_argument

    # Program functionality arguments.
    addarg('-a', '--author', required=False)
    addarg('-t', '--title', required=False)
    addarg('-p', '--publisher', required=False)
    addarg('-y', '--year', type=int, required=False)
    addarg('-l', '--language', required=False,
            help='Two letters denoting the item\'s language. e.g. \'en\' for English or \'sv\' for Swedish')
    addarg('-e', '--extension', required=False,
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
        sys.exit(1)

    wanted = Item(args)
    found = libgen.get_results(wanted)

    output = (
        "Found %d results!" % len(found),
        "title: %s" % found[0].title,
        "author: %s" % found[0].author,
        "publisher: %s" % found[0].publisher,
        "year: %s" % found[0].year,
        "language: %s" % found[0].lang,
        "extension: %s" % found[0].ext
    )

    for line in output:
        print(line)

    # print all results here

#    query = args.query
#
#    for source in sources:
#        r = search(source, query)

if __name__ == '__main__':
    sys.exit(main(sys.argv))

