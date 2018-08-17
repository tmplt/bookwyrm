import pybookwyrm as bw

def find(wanted, bookwyrm):
    # Construct empty fields
    # TODO: streamline this: can all three fields just be dicts?
    nonexacts = bw.nonexacts_t({}, [])
    exacts = bw.exacts_t({}, '')
    misc = bw.misc_t([], [])

    book = (nonexacts, exacts, misc)
    bookwyrm.feed(book)

#PASS trying to add one new item...
