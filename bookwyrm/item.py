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

from enum import Enum, unique
from fuzzywuzzy import fuzz
from collections import namedtuple
import argparse
import itertools

FUZZ_RATIO_DEF = 75

# A namedtuple, but with optional arguments.
# Credits:
# <https://stackoverflow.com/questions/11351032/named-tuple-and-optional-keyword-arguments/16721002#16721002>
class OptNamedTuple(namedtuple('OptNamedTuple', ['year', 'lang', 'edition', 'doi', 'ext'])):
    __slots__ = ()
    def __new__(cls, year=None, lang=None, edition=None, doi=None, ext=None):
        return super(OptNamedTuple, cls).__new__(cls, year, lang, edition, doi, ext)

class Data(namedtuple('Data', ['authors', 'title', 'publisher', 'isbns', 'mirrors'])):
    __slots__ = ()
    def __new__(cls, authors, title, publisher, isbns, mirrors=None):
        return super(Data, cls).__new__(cls, authors, title, publisher, isbns, mirrors)

class Item:
    """A class to hold all data for a book or paper."""

    def __init__(self, *args):
        if len(args) == 1 and isinstance(args[0], argparse.Namespace):
                self.init_from_argparse(args[0])
        elif len(args) == 2: # check type
            self.data = args[0]
            self.exacts = args[1]

    def init_from_argparse(self, args):
        self.data = Data(
            authors = args.author,
            title = args.title,
            publisher = args.publisher,
            isbns = args.isbn
        )

        self.exacts = OptNamedTuple(
            year = args.year,
            lang = args.language,
            edition = args.edition,
            doi = args.doi,
            ext = args.extension
        )

    def matches(self, wanted):

        # Parallell iteration over the two tuples of exact values.
        for val, wnt in zip(self.exacts, wanted.exacts):
            if wnt is not None:
                return val == wnt

        if wanted.data.isbns:
            try:
                if not set(wanted.data.isbns).intersection(self.data.isbns):
                    return False
            except TypeError:
                return False

        # partial_ratio, useful for course literature which can have some
        # crazy long titles.
        if wanted.data.title:
            ratio = fuzz.partial_ratio(self.data.title, wanted.data.title)
            if ratio < FUZZ_RATIO_DEF:
                return False

        # token_set seems to work best here, both when only
        # the last name is given but also when something like
        # "J. Doe" is given.
        if wanted.data.authors:
            ratio_thus_far = 0
            for comb in itertools.product(self.data.authors, wanted.data.authors):
                fuzz_ratio = fuzz.token_set_ratio(comb[0], comb[1])
                ratio_thus_far = max(fuzz_ratio, ratio_thus_far)

            if ratio_thus_far < FUZZ_RATIO_DEF:
                return False

        # We use partial ratio here since some sources may not use the
        # publisher's full name.
        if wanted.data.publisher:
            ratio = fuzz.partial_ratio(self.data.publisher, wanted.data.publisher)
            if ratio < FUZZ_RATIO_DEF:
                return False

        return True

