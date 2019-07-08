#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# This file is part of bookwyrm

"""
Queries existing local Library Genesis <https://en.wikipedia.org/wiki/Library_Genesis> databases for item matches.

Strategy: conditions use the `LIKE %substring%` operator on major columns (authors, title, series, publisher)
to narrow results down. All rows are then fed to bookwyrm for final selection.
"""

import os
import re
import sqlite3
import isbnlib
import threading
import itertools
import concurrent.futures
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


def _max(a, b):
    if not a and not b: return None
    if not b: return a
    if not a: return b
    return max(a, b)


def extract_valid_isbns(isbns: str) -> [str]:
    return [isbnlib.mask(isbn) for isbn in isbns if isbnlib.is_isbn10(isbn) or isbnlib.is_isbn13(isbn)]


def libgen(wanted, bookwyrm):
    c = _open_db("libgen.db")
    query = "SELECT * FROM updated WHERE "
    conditions = []
    fields = tuple()

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

    # Unused schema fields:
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
            'isbns': extract_valid_isbns(row['Identifier'].split(',')),
            # NOTE(maybe): 'Year' column is a string here
            'year': _int(row['Year']) if maybe(_int(row['Year'])) > 0 else None,
            'volume': _int(maybe(re.search(r'\d+', row['VolumeInfo'])).group()),
        }
        bookwyrm.feed(item)


def fiction(wanted, bookwyrm):
    c = _open_db("fiction.db")
    for table in ('main', 'main_edited'): # XXX: how do they differ?
        query = f"SELECT * FROM {table} WHERE "
        conditions = []
        fields = tuple()

        # Narrow the search with any main arguments;
        # let bookwyrm handle the rest.

        # Authors are separated into columns of `AuthorFamily<i>`, `AuthorName<i>` and `AuthorSurname<i>`
        # where `i = range(1, 4)`. This schema is not necessarily respected by all entries, so we must split
        # the wanted authors name into first and last, and generate the product of all possible column-value
        # combinations. E.g.:
        #   ('AuthorName1', firstname) ('AuthorName1', lastname) ('AuthorFamily1', firstname) ...
        if 'authors' in wanted:
            cols = sum([[f"AuthorFamily{i}", f"AuthorName{i}", f"AuthorSurname{i}"] for i in range(1, 4)], [])
            for author in wanted['authors']:
                for p in itertools.product(cols, author.split()): # XXX: should we strip '.'?
                    col, val = p
                    conditions.append(f"{col} LIKE ?")
                    fields += (f"%{val}%",)

        if 'series' in wanted:
            for col in [f"Series{i}" for i in range(1, 5)]:
                conditions.append(f"{col} LIKE ?")
                fields += (f"%{wanted['series']}%",)


        for key in ['title', 'publisher']:
            if (key in wanted):
                conditions.append(f"{key.title()} LIKE ?")
                fields += (f"%{wanted[key]}%",)

        query += " OR ".join(conditions)
        bookwyrm.log.debug(query)

        # Unused schema fields:
        # - Pseudonim{1-4}
        # - Role{1-4}
        # - Version
        # - Path
        # - Commentary
        # - TimeAdded
        # - TimeLastModified
        # - RussianAuthorFamily
        # - RussianAuthorName
        # - RussianAuthorSurname
        # - Cover
        # - GooglebookID
        # - ASIN
        # - AuthorHash
        # - TitleHash
        # - Visible

        for row in c.execute(query, fields):
            # Same author problem as above, but now we have to reverse it:
            # find all (Name, Surname, Family) tuples and format each tuple if at least one column is non-empty.
            authors_cols = [
                (row[f'AuthorName{i}'], row[f'AuthorSurname{i}'], row[f'AuthorFamily{i}'])
                for i in range(1, 5) if
                any((row[f'AuthorName{i}'], row[f'AuthorSurname{i}'], row[f'AuthorFamily{i}']))
            ]
            authors = [" ".join([e.strip() for e in tup if e != '']) for tup in authors_cols]

            # XXX: we only consider the first non-empty series column
            series = next((s.strip() for s in (row['Series1'], row['Series2'], row['Series3'], row['Series4']) if s.strip() != ''), None)

            item = {
                'title': row['Title'].strip(),
                'edition': _int(row['Edition']),
                'series': series,
                'publisher': row['Publisher'].strip(),
                'language': row['Language'].strip(),
                'authors': authors,
                'pages': _int(row['Pages']),
                'size': row['Filesize'],
                'extension': row['Extension'].strip(),
                'mirrors': [f"http://booksdescr.org/foreignfiction/ads.php?md5={row['MD5'].strip()}"],
                'coverurl': f"http://booksdescr.org/fictioncovers/32000/{row['MD5'].strip()}.jpg",
                'isbns': extract_valid_isbns(row['Identifier'].split(',')),
                # NOTE(maybe): 'Year' column is a string here
                'year': _int(row['Year']) if maybe(_int(row['Year'])) > 0 else None,
            }
            bookwyrm.feed(item)


def resolve(mirror: str) -> (str, dict):
    if 'booksdescr.org/foreignfiction' in mirror:
        return (mirror, {})


def find(wanted, bookwyrm):
    tasks = [libgen, fiction]
    with concurrent.futures.ThreadPoolExecutor(max_workers=len(tasks), thread_name_prefix='bw/libgen') as executor:
        fs = {executor.submit(task, wanted, bookwyrm): task for task in tasks}
        concurrent.futures.wait(fs, return_when=concurrent.futures.FIRST_EXCEPTION)
        for f in concurrent.futures.as_completed(fs):
            # Raise any exceptions to bookwyrm
            f.result()


if __name__ == "__main__":
    print(XDG_DATA_HOME)
    conn = sqlite3.connect(str(XDG_DATA_HOME / "bookwyrm" / "libgen.db"))
    conn.row_factory = sqlite3.Row
    c = conn.cursor()
    c.execute("SELECT * FROM updated WHERE VolumeInfo <> ''")
    for i in range(10):
        r = c.fetchone()
        code.interact(local=locals())
        print(f"{r['ID']}: {r['Title']}")
