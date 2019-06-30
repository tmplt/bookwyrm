#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# This file is part of bookwyrm

"""
Scrapes Library Genesis <https://en.wikipedia.org/wiki/Library_Genesis> for item matches.
"""

import pybookwyrm as bw

import os
import re
import sqlite3
import isbnlib
from pathlib import Path
from pymaybe import maybe
from timeit import default_timer as timer

HOME = Path(os.path.expandvars("$HOME"))
XDG_DATA_HOME = os.environ.get("XDG_DATA_HOME") or (HOME / ".local" / "share")

def _open_db(db: str) -> sqlite3.Cursor:
    conn = sqlite3.connect(str(XDG_DATA_HOME / "bookwyrm" / db))
    conn.row_factory = sqlite3.Row
    return conn.cursor()

def _int(s: str):
    try:
        return int(s)
    except (ValueError, TypeError):
        return None

def _max(a, b) -> int:
    if not b: return a
    if not a: return b
    return max(a, b)

def find(wanted, bookwyrm):
    c = _open_db("libgen.db")
    query = "SELECT * FROM updated WHERE "
    conditions = []
    fields = tuple()

    # Narrow the search with any main arguments;
    # let bookwyrm handle the rest.

    if ('authors' in wanted):
        for author in wanted['authors']:
            conditions.append('Author LIKE ?')
            fields += (f"%{author}%",)

    for key in ['title', 'series', 'publisher']:
        if (key in wanted):
            conditions.append(f"{key.title()} LIKE ?")
            fields += (f"%{wanted[key]}%",)

    query += " OR ".join(conditions)
    bookwyrm.log.debug(query)

    # XXX: unused schema fields:
    # - Periodical
    # - City
    # - Topic
    # - Library
    # - Issue
    # - ISSN, ASIN, UDC, LBC, DDC, LCC, Doi, Googlebookid, OpenlibraryID
    # - Commentary
    # - DPI
    # - Color
    # - Cleaned
    # - Orientation
    # - Paginated
    # - Scanned
    # - Bookmarked
    # - Searchable
    # - Generic
    # - Visible
    # - Locator
    # - Local
    # - TimeAdded
    # - TimeLastModifed
    # - Coverurl
    # - Tags
    # - IndetifierWODash

    # there is only data in `updated`
    for row in c.execute(query, fields):
        t1 = timer()
        item = {
            'title': row['Title'].strip(),
            'edition': _int(row['Edition']),
            'series': row['Series'].strip(),
            'publisher': row['Publisher'].strip(),
            'language': row['Language'].strip(),
            'authors': [row['Author'].strip()],
            'pages': _max(row['PagesInFile'], _int(row['Pages'])),
            'size': _int(row['Filesize']),
            'extension': row['Extension'].strip(),
            'mirrors': [f"http://booksdescr.org/ads.php?md5={row['MD5'].strip()}"],
            'coverurl': f"http://booksdescr.org/covers/{row['Coverurl']}",
            'isbns': [isbnlib.mask(isbn) for isbn in row['Identifier'].split(',') if isbnlib.is_isbn10(isbn) or isbnlib.is_isbn13(isbn)],
            'year': _int(row['Year']),
            'volume': _int(maybe(re.search(r'\d+', row['VolumeInfo'])).group()),
        }
        t2 = timer()
        bookwyrm.feed(item)
        bookwyrm.log.debug(f"item ctr took {t2 - t1}s; feed took {timer() - t2}s")
        bookwyrm.log.debug("".join(item['isbns']))


if __name__ == "__main__":
    print(XDG_DATA_HOME)
    conn = sqlite3.connect(str(XDG_DATA_HOME / "bookwyrm" / "libgen.db"))
    conn.row_factory = sqlite3.Row
    c = conn.cursor()
    c.execute("SELECT * FROM updated WHERE VolumeInfo <> ''")
    for i in range(10):
        r = c.fetchone()
        # code.interact(local=locals())
        print(f"{r['ID']}: {r['Title']}")
