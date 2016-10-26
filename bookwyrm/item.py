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
import argparse

FUZZ_RATIO_DEF = 75

# Use <https://docs.python.org/3/library/enum.html#orderedenum>
# for specified priority?
@unique
class ext(Enum):
    pdf = 1
    epub = 2
    djvu = 3

class Item:
    """A class to hold att data for a book or paper."""

    def __init__(self, *args):
        self.authors = None
        self.title = None
        self.publisher = None
        self.year = None
        self.lang = None
        self.isbn = None
        self.edition = None
        self.doi = None
        self.ext = None

        mirrors = None

        if len(args) == 1 and isinstance(args[0], argparse.Namespace):
                self.init_from_argparse(args[0])
        elif len(args) > 1:
            raise NotImplementedError('invalid initialization method')

    def init_from_argparse(self, args):
        self.authors = args.author
        self.title = args.title
        self.publisher = args.publisher
        self.year = args.year
        self.lang = args.language.lower() if args.language else None
        self.isbn = args.isbn
        self.edition = args.edition
        self.doi = args.doi
        self.ext = args.extension

    def __eq__(self, wanted):
        # Exact values
        # NOTE: mention these as 'exact' in docs.
        # NOTE: is this the right place for lang?
        if ((wanted.year and self.year != wanted.year) or
                (wanted.lang and self.lang != wanted.lang) or
                (wanted.edition and self.edition != wanted.edition) or
                (wanted.doi and self.doi != wanted.doi) or
                (wanted.ext and self.ext != wanted.ext)):
            return False

        if (wanted.isbn and wanted.isbn not in self.isbn):
            return False

        # Useful for course literature which can have some
        # crazy long titles.
        if wanted.title:
            ratio = fuzz.partial_ratio(self.title, wanted.title)
            if ratio < FUZZ_RATIO_DEF:
                return False

        # token_set seems to work best here, both when only
        # the last name is given but also when something like
        # "J. Doe" is given.
        if wanted.authors:
            ratio_thus_far = 0
            for author in self.authors:
                fuzz_ratio = fuzz.token_set_ratio(author, wanted.authors)
                ratio_thus_far = max(fuzz_ratio, ratio_thus_far)

            if ratio_thus_far < FUZZ_RATIO_DEF:
                return False

        # Often short; no need to do anything special.
        if wanted.publisher:
            ratio = fuzz.ratio(self.publisher, wanted.publisher)
            if ratio > FUZZ_RATIO_DEF:
                return False

        return True

