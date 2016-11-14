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
import argparse
import itertools

FUZZ_RATIO_DEF = 75


class Exacts(namedtuple('Exacts', ['year', 'lang', 'edition', 'ext'])):
    """A named tuple, but with optional arguments."""
    __slots__ = ()

    def __new__(cls, year=None, lang=None, edition=None, ext=None):
        return super(Exacts, cls).__new__(cls, year, lang, edition, ext)

Data = namedtuple('Data', ['authors', 'title', 'serie', 'publisher',
                           'isbns', 'mirrors', 'exacts'])


class Item:
    """Holds all data for a item."""

    def __init__(self, arg):
        if isinstance(arg, argparse.Namespace):
            self.__init_from_argparse(arg)
        elif isinstance(arg, Data):
            self.data = arg

    def __init_from_argparse(self, args):

        self.data = Data(
            authors = args.author,
            title = args.title,
            serie = args.serie,
            publisher = args.publisher,
            isbns = args.isbn,

            # Since Data's attributes aren't optional and using a
            # named tuple with optional arguments made for some
            # wierd behavior.
            mirrors = None,

            exacts = Exacts(
                year = args.year,
                lang = args.language,
                edition = args.edition,
                ext = args.extension
            )
        )

    def __getattr__(self, key):
        return getattr(super(Item, self).__getattribute__('data'), key)

    def matches(self, wanted, fuzzy_min=FUZZ_RATIO_DEF):
        """
        Returns true if all specified exact values are equal
        and if all specified non-exact values pass the fuzzy ratio.
        """

        def match_partial(s1, s2):
            ratio = fuzz.partial_ratio(s1, s2)
            return ratio >= fuzzy_min

        # Parallell iteration over the two tuples of exact values.
        for val, req in zip(self.exacts, wanted.exacts):
            if req is not None:
                if val != req:
                    return False

        if wanted.isbns:
            if not set(wanted.isbns) & set(self.isbns):
                return False

        in_result = (self.title, self.serie, self.publisher)
        requested = (wanted.title, wanted.serie, wanted.publisher)

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
        if wanted.authors:
            ratio_thus_far = 0
            for comb in itertools.product(self.data.authors,
                                          wanted.data.authors):
                ratio = fuzz.token_set_ratio(comb[0], comb[1])
                ratio_thus_far = max(ratio, ratio_thus_far)

            if ratio_thus_far < fuzzy_min:
                return False

        return True
