import pybookwyrm as bw
import time


def find(wanted, bookwyrm):
    # Changes to wanted are module-local
    # since the item is copied.
    wanted.nonexacts.title = "new title"

    # Generate some dummy items
    for i in range(100):
        # time.sleep(0.1)
        nonexacts = bw.nonexacts_t({
            'title': 'Some Title (' + str(i) + ')',
            'serie': 'The Cool Serie' + str(i),
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
            }
        )

        book = (nonexacts, exacts)
        bookwyrm.feed(book)
