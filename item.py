# coding: utf-8

from enum import Enum, unique
from bs4 import BeautifulSoup as bs
import libgen

entities = (
    'author',
    'title',
    'year',
    'language',
    'size',
    'ext',
    'isbn',
    'doi'
)

# Use <https://docs.python.org/3/library/enum.html#orderedenum>
# for specified priority?
@unique
class ext(Enum):
    pdf = 1
    epub = 2
    djvu = 3

class item(object):
    author = None
    title = None
    publisher = None
    year = None
    language = None
    size = None
    ext = None
    isbn = None
    doi = None

    # lets not do this.
    # lets instead only use this class for found items
    def __new__(self, args):
        # check for <class 'argparse.Namespace'>
        pass

    def __new__(self, html_row):
        if html_row.__name__ != "Tag":
            raise NotImplementedError('items can only be initialized from BeautifulSoup tags')

        # do the thing with bs
        pass

    # Do we want pages and ID too?


