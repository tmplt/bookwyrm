import pybookwyrm

# Construct an item, but with an invalid type for the authors field
def find(wanted, bookwyrm):
    book = {'authors': None}
    bookwyrm.feed(book)

#PASS error: plugin 'invalid-authors' exited non-successfully: TypeError("'NoneType' object is not iterable",)
