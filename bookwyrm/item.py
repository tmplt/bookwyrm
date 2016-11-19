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

from fuzzywuzzy import fuzz
from collections import namedtuple
from enum import Enum
import argparse
import itertools

FUZZ_RATIO_DEF = 75

NONEXACTS = [
    'authors',
    'title',
    'serie',
    'publisher',
    'journal'
]

EXACTS = [
    'year',
    'lang',
    'edition',
    'ext',
    'volume',
    'number',
    'pages'
]

MISC = [
    'isbns',
    'mirrors'
]


class NonExacts(namedtuple('NonExacts', NONEXACTS)):
    __slots__ = ()

    def __new__(cls, authors, title, serie=None, publisher=None, journal=None):
        return super(NonExacts, cls).__new__(cls, authors, title, serie, publisher, journal)


class Exacts(namedtuple('Exacts', EXACTS)):
    __slots__ = ()

    def __new__(cls, year=None, lang=None, edition=None, ext=None, volume=None, number=None, pages=None):
        return super(Exacts, cls).__new__(cls, year, lang, edition, ext, volume, number, pages)


class Misc(namedtuple('Misc', MISC)):
    __slots__ = ()

    def __new__(cls, isbns=None, mirrors=None):
        return super(Misc, cls).__new__(cls, isbns, mirrors)


class ItemType(Enum):
    paper = 0
    book = 1


class Item:
    """Holds all data for a item."""

    nonexacts = NonExacts
    exacts = Exacts
    misc = Misc

    def __init__(self, namespace=None, nonexacts=None, exacts=None, misc=None):
        if isinstance(namespace, argparse.Namespace):
            self.__init_from_argparse(namespace)
        else:
            self.nonexacts = nonexacts
            self.exacts = exacts
            self.misc = misc

    def __init_from_argparse(self, args):
        self.nonexacts = NonExacts(
            authors = args.author,
            title = args.title,
            serie = args.serie,
            publisher = args.publisher,
            # journal = args.journal
        )

        self.exacts = Exacts(
            # year = args.year,
            # edition = args.edition,
            # ext = args.extension,
            # volume = args.volume,
            # number = args.number
        )

        self.misc = Misc(
            # isbns = args.isbns,
            mirrors = None
        )

    # def __getattr__(self, key):
    #     return getattr(super(Item, self).__getattribute__('data'), key)

    def item_type(self):
        paper_exclusive = (
            self.nonexacts.journal,
            self.exacts.volume,
            self.exacts.number
        )

        if any(paper_exclusive):
            return ItemType.paper

        return ItemType.book

    def matches(self, wanted, fuzzy_min=FUZZ_RATIO_DEF):
        """
        Returns true if all specified exact values are equal
        and if all specified non-exact values pass the fuzzy ratio.
        """

        def match_partial(s1, s2):
            ratio = fuzz.partial_ratio(s1, s2)
            return ratio >= fuzzy_min

        # Don't check if a Paper matches a Book and vise-versa.
        if not self.item_type() is wanted.item_type():
            return False

        # Parallell iteration over the two tuples of exact values.
        for val, req in zip(self.exacts, wanted.exacts):
            if req is not None:
                if val != req:
                    return False

        try:
            if wanted.misc.isbns:
                if not set(wanted.misc.isbns) & set(self.misc.isbns):
                    return False
        except TypeError:
            # Book.isbns exists, but Paper.isbns doesn't.
            pass

        in_result = (self.nonexacts.title, self.nonexacts.serie, self.nonexacts.publisher)
        requested = (wanted.nonexacts.title, wanted.nonexacts.serie, wanted.nonexacts.publisher)

        for val, req in zip(in_result, requested):
            if req is not None:
                # partial: useful for course literature which can have some
                # crazy long titles. Also useful for publisher,
                # because some entries may not use the full name of it.
                if not match_partial(val, req):
                    return False

        # token_set seems to work best here, both when only
        # the last name is given but also when something like
        # "J. Doe" is given.
        if wanted.nonexacts.authors:
            ratio_thus_far = 0
            for comb in itertools.product(self.nonexacts.authors, wanted.nonexacts.authors):
                ratio = fuzz.token_set_ratio(comb[0], comb[1])
                ratio_thus_far = max(ratio, ratio_thus_far)

            if ratio_thus_far < fuzzy_min:
                return False

        return True
