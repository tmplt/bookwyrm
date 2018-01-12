import pybookwyrm as bw
import time


def find(wanted, bookwyrm):
    # Changes to wanted are module-local
    # since the item is copied.
    # wanted.nonexacts.title = "new title"

    # Generate some dummy items
    for i in range(100):
        nonexacts = bw.nonexacts_t({
            'title': 'Some Title (' + str(i) + ')',
            'series': 'The Cool Series' + str(i),
            'publisher': 'Fuck Pearson',
            'journal': 'No journal, no'
            },

            ['Author A. ' + str(i), 'Author B.' + str(i)]
        )

        exacts = bw.exacts_t({
            'year': 2000 + i,
            'edition': i,
            'volume': i,
            'number': 30 + i,
            'pages': 500 + i,
            },
            'pdf'
        )

        misc = bw.misc_t([
            'http://localhost:8000/big',
            'http://localhost:8000/invalidurl.txt',
            'http://localhost:8000/helloworld.txt'
        ], ['isbn1', 'isbn2'])

        book = (nonexacts, exacts, misc)
        bookwyrm.feed(book)
