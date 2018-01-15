import pybookwyrm as bw
import time


def find(wanted, bookwyrm):
    # Changes to wanted are module-local
    # since the item is copied.
    # wanted.nonexacts.title = "new title"

    # Generate some dummy data
    for i in range(10):
        time.sleep(0.2)

        nonexacts = bw.nonexacts_t(
            {'series': 'series' + str(i), 'title': 'Some Title (' + str(i) + ')'},
            ['Author A. ' + str(i), 'Author B.' + str(i)])

        exacts = bw.exacts_t({'year': 2000 + i, 'pages': 500 + i}, 'pdf')

        misc = bw.misc_t(['http://localhost:8000/helloworld.txt'], ['isbn'])

        book = (nonexacts, exacts, misc)
        bookwyrm.feed(book)
