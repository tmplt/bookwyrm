#!/usr/bin/env python

import requests
from bs4 import BeautifulSoup as bs
import string
from enum import IntEnum

class column(IntEnum):
    id = 0
    author = 1
    title = 2
    publisher = 3
    year = 4
    # skip page count column
    lang = 6
    ext = 8

r = requests.get('http://libgen.io/?req=temeraire')
soup = bs(r.text, 'html.parser')

results = []
for row in soup.find_all('tr'):
    if row.find('td').text.isdigit():
        results.append(row)

a = results[0].find_all('td')[column.publisher]
