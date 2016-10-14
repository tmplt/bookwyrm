#!/usr/bin/env python

import requests
from bs4 import BeautifulSoup as bs
import string
from enum import IntEnum
from bookwyrm import utils

class column(IntEnum):
    id = 0
    author = 1
    title = 2
    isbns = 2
    publisher = 3
    year = 4
    # skip page count column
    lang = 6
    ext = 8

# r = requests.get('http://libgen.io/?req=calculus+a+complete+course')

# soup = bs(r.text, 'html.parser')

# results = []
# for row in soup.find_all('tr'):
#     if row.find('td').text.isdigit():
#         results.append(row)

# a = results[0].find_all('td')[column.isbns]

r = requests.get('http://libgen.io/book/index.php?md5=556777D5F48051787ED86DC50EE72C9F&oftorrent=')
magnet = utils.magnet_from_torrent(r.content)
print(magnet)

