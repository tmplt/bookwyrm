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

from enum import Enum
import sys
import argparse
import requests
import logging

from item import Item
from utils import eprint, ExitCode
import libgen
import scihub
import gscholar
import utils


# NOTE: enable the user to config there.
class Sources(Enum):
    libgen = 0
    gscholar = 1

    # Not yet implemented sources:
    # irc = 3
    # torrents = 4

SEARCH_TABLE = {
    Sources.libgen: libgen.search,
    Sources.gscholar: gscholar.search
}


class IdentType(Enum):
    direct = 0
    paywall = 1
    doi = 2


class Bookwyrm:

    def __init__(self, args, logger):
        self.args = args
        self.wanted = Item(namespace=args)
        self.logger = logger

        self.count = 0
        self.results = []

    def __enter__(self):
        self.logger.debug('summoning the mighty, all-knowing bookwyrm!')
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.logger.debug('killed the bookwyrm (:c) with exc_type, exc_value = (%s, %s)'
                          % (exc_type, exc_value))

    def print_items(self):
        for idx, item in enumerate(self.results):
            edition = item.exacts.edition
            ext = item.exacts.ext

            line = "%d | %s, %s, %s" % (
                idx,
                item.nonexacts.title,
                utils.ordinal_num(edition) + " ed." if edition else "n/a ed.",
                ext
            )

            print(line)

    def _filter_unwanted(self, wanted, lst):
        self.logger.debug('filtering items to match wanted values')

        # NOTE: are we eq'ing items twice here?
        for item in lst[:]:
            if not item.matches(wanted):
                lst.remove(item)

        return lst

    def search(self, source):
        """Search all sources for an item matching what's wanted."""

        unprocessed_results = SEARCH_TABLE[source](self.wanted)
        self.results += self._filter_unwanted(self.wanted, unprocessed_results)

        self.count += len(self.results)  # used in main()

    def fetch(self, ident):
        """Download (fetch) an item from an identifyer."""

        url = self._get_direct_url(ident)
        r = requests.get(url)

        return {
            'pdf': r.content,
            'url': url,
            'name': scihub.generate_name(r)
        }

    def _get_direct_url(self, ident):
        id_type = self._classify(ident)
        self.logger.debug('\'%s\' classified as %s' % (ident, id_type))

        if id_type == IdentType.direct:
            return ident
        else:
            return scihub.search_direct_url(ident)

    def _classify(self, ident):
        if utils.valid_doi(ident):
            return IdentType.doi
        elif ident.endswith('http'):
            # NOTE: Of course, direct links to a .pdf can end with something else.
            if ident.endswith('pdf'):
                return IdentType.direct
            else:
                return IdentType.paywall
        else:
            raise ValueError('invalid ident')


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
    add_optarg('--version', action='version', version='%(prog)s 0.3.0-alpha')
    add_optarg('--debug', action='store_true')

    args = parser.parse_args()
    validate_arguments(args, parser)

    if args.debug:
        logger.setLevel(logging.DEBUG)

    return args


def validate_arguments(args, parser):
    required_arg = (
        args.author,
        args.title,
        args.serie,
        args.publisher,
        args.ident
    )

    if not any(required_arg) and len(sys.argv) > 2:
        parser.error('missing necessarily inclusive argument.')

    elif any(required_arg[:-1]) and args.ident:
        parser.error('ident flag is exclusive, and may not be passed with another flag.')


def main(logger):
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

    with Bookwyrm(args, logger) as bw:
        if args.ident:
            logger.debug('ident specified.')

            try:
                pdf = bw.fetch(args.ident)
            except ValueError:
                eprint('I couldn\'t find anything.')
                return ExitCode.no_results_found

            logger.debug('generated filename \'%s\'' % pdf['name'])

            logger.debug('writing to disk...')
            utils.write(pdf)

            return ExitCode.success

        for source in Sources:
            logger.debug('traversing %s...' % source)
            bw.search(source)

        if bw.count > 0:
            print('I found %d items!' % bw.count)
            bw.print_items()
        else:
            eprint('I couldn\'t find anything.')
            return ExitCode.no_results_found

        return ExitCode.success


if __name__ == '__main__':
    logging.basicConfig(format='%(levelname)s:%(name)s:%(funcName)s: %(message)s')
    logger = logging.getLogger('bookwyrm')

    retval = main(logger)
    logger.debug('reached termination, exit code = %d' % retval)
    sys.exit(retval)
