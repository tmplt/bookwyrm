import pybookwyrm as bw


def test():
    print("hello world (from Python)")

    # Example data
    serie = "Temeraire"
    title = "League of Dragons"
    authors = ["Naomi Novik"]
    nonexacts = bw.nonexacts_t({'serie': serie, 'title': title}, authors)
    print(nonexacts)

    year = 2138
    pages = 380
    exacts = bw.exacts_t({'year': year, 'pages': pages})
    print(exacts)

    print("returning...")
    return nonexacts, exacts
