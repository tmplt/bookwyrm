import pybookwyrm as bw

def find(wanted, bookwyrm):
    # Construct empty fields
    # TODO: streamline this: can all three fields just be dicts?
    # nonexacts = bw.nonexacts_t({}, [])
    # exacts = bw.exacts_t({}, '')
    # misc = bw.misc_t([], [])

    book = {
        'year': 2038,
        'pages': 531,
        'title': 'some title',
        'authors': ['A', 'B', 'C'],
    }
    bookwyrm.feed(book)

#PASS trying to add one new item with title 'some title'...
