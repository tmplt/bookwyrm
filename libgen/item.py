# coding: utf-8

from enum import Enum, unique
from bs4 import BeautifulSoup as bs

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

    def __new__(self, html_row):
        # do the thing with bs
        pass

    # Do we want pages and ID too?
