import pybookwyrm

# Construct an item, but with an invalid type for the authors field
def find(wanted, bookwyrm):
    book = {'authors': None}
    bookwyrm.feed(book)

#PASS TypeError("'NoneType' object is not iterable",)
