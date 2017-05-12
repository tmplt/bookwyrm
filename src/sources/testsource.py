import pybookwyrm as bw


def find(wanted, bookwyrm):
    # Changes to wanted are module-local
    # since the item is copied.
    wanted.nonexacts.title = "new title"

    # Generate some dummy data
    for i in range(10):
        nonexacts = bw.nonexacts_t(
            {'serie': 'serie' + str(i), 'title': 'title' + str(i)},
            ['Author A. ' + str(i), 'Author B.' + str(i)])

        exacts = bw.exacts_t({'year': 2000 + i, 'pages': 500 + i})

        book = (nonexacts, exacts)
        bookwyrm.feed(book)

    print(__name__, "done, returning nothing...")
