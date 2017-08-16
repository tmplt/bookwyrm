import pybookwyrm as bw
import time


def find(wanted, bookwyrm):
    # Changes to wanted are module-local
    # since the item is copied.
    wanted.nonexacts.title = "new title"

    # Generate some dummy data
    for i in range(10):

        if bookwyrm.terminating():
            return

        time.sleep(0.2)

        nonexacts = bw.nonexacts_t(
            {'serie': 'serie' + str(i), 'title': wanted.nonexacts.title + str(i)},
            ['Author A. ' + str(i), 'Author B.' + str(i)])

        exacts = bw.exacts_t({'year': 2000 + i, 'pages': 500 + i})

        book = (nonexacts, exacts)
        bookwyrm.feed(book)
