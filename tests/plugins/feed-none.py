def find(wanted, bookwyrm):
    book = {
        'series': None,
        'publisher': None,
        'language': None,
        'authors': None,
        'pages': None,
        'size': None,
        'extension': None,
        'mirrors': None,
        'title': None,
        'edition': None,
        'isbns': None,
        'year': None,
    }

    bookwyrm.feed(book)

#PASS item not a match close enough, or missing title/URI
