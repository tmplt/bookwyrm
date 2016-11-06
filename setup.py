#!/usr/bin/env python
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

import os
import sys
from setuptools import setup


def _read(fn):
    path = os.path.join(os.path.dirname(__file__), fn)
    return open(path).read()

setup(
    name='bookwyrm',
    version='0.1.0',
    description='book/paper finder and retriever',
    author='Tmplt',
    author_email='ttemplate223@gmail.com',
    license='MIT',
    long_description=_read('README.md'),

    packages=[
        'bookwyrm',
        'bookwyrm.libgen'
    ],

    entry_points={
        'console_scripts': [
            'bookwyrm = bookwyrm.bookwyrm:main',
        ],
    },

    install_requires=[
        'argparse',
        'requests',
        'urllib3',
        'bencodepy',
        'fuzzywuzzy',
        'furl'
    ] + (['enum34>=1.0.4'] if sys.version_info < (3, 4, 0) else []),

    classifiers=[
        'Development Status :: 1 - Pre-Alpha',
        'Environment :: Console',
        'License :: OSI Approved :: MIT License',
        'Operating System :: POSIX',
        'Programming Lanugage :: Python :: 3',
        # 'Topic :: ???'
    ],
)
