#!/usr/bin/env python

import requests
from bs4 import BeautifulSoup as bs
import string

def func(tag):
    print("%s, %s\n\n" % (tag.name, tag.text))
    try:
        print(tag.find('id'))

        return tag.name == 'a' and tag.text.isdigit()
    except AttributeError:
        return False

def func2(s):
    print(s)
    try:
        return s.isdigit()
    except AttributeError:
        return False

r = requests.get('http://libgen.io/?req=temeraire')
soup = bs(r.text, 'html.parser')

results = []
for row in soup.find_all('tr'):
    if row.find('td').text.isdigit():
        results.append(row)

a = results[0].find_all('td')[2]
#b = a.find('a', attrs={'id': string.isdigit})
a.find(id=func2)
print(a)
