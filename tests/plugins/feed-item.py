import pybookwyrm as bw

def find(wanted, bookwyrm):
    book = {
        'year': 2038,
        'pages': 531,
        'title': 'some title',
        'authors': ['A', 'B', 'C'],
    }
    bookwyrm.feed(book)

#PASS trying to add one new item with title 'some title'...
