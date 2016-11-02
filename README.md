## What is this
Give `bookwyrm` some data which relates to a book, paper or similar.
With this data, it'll look trough all manners of sources (see [here](#planned-supported-sources)),
find some URIs, and then use an external downloader of your choice for processing.
By default, `bookwyrm` might use `aria2c`,
or perhaps something that all systems have at boot.
Not sure yet.

## Usage
```
usage: bookwyrm [-h] [-a AUTHOR] [-t TITLE] [-p PUBLISHER] [-y YEAR]
                [-l LANGUAGE] [-e EDITION] [-E EXTENSION] [-i ISBN]
                [-d DOI] [-u URL] [-v] [--version]

bookwyrm - find books and papers online and download them. When called with
no arguments, bookwyrm prints this screen and exits.

optional arguments:
  -h, --help            show this help message and exit
  -a AUTHOR, --author AUTHOR
  -t TITLE, --title TITLE
  -s SERIE, --serie SERIE
  -p PUBLISHER, --publisher PUBLISHER
  -y YEAR, --year YEAR
  -l LANGUAGE, --language LANGUAGE
  -e EDITION, --edition EDITION
  -E EXTENSION, --extension EXTENSION
                        filename extension without period, e.g. 'pdf'.
  -i ISBN, --isbn ISBN
  -d DOI, --doi DOI
  -u URL, --url URL
  -v, --verbose         verbose mode; prints out a lot of debug information.
                        Can be used more than once, e.g. -vv, to increase the
                        level of verbosity.
  --version             show program's version number and exit
```

## Planned supported sources
### High priority
* Library Genesis
* Bookzz and Booksc
* Sci-Hub
* IRC channels
    - <p>#ebooks on irc-highway</p>
    - etc.
### Medium priority
* Various torrent sides
    - Private (bB, biblotik, etc.)
    - Torrent Project
    - etc.


## Notes
* If an URL for an academic paper is ever needed, [dx.doi.org](http://dx.doi.org) can be used.
