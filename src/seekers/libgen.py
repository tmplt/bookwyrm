#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# This file is part of bookwyrm

# Copyright (C) 2017 Tmplt <tmplt@dragons.rocks>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""
Scrapes Library Genesis <https://en.wikipedia.org/wiki/Library_Genesis> for item matches.
"""

from bs4 import BeautifulSoup as bs

DOMAINS = ('libgen.io', 'gen.lib.rus.ec')

if __name__ == "__main__":
    print('Hello World!')
