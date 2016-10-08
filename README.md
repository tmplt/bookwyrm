## What is this?
The town's local bookwyrm has a vast library
ranging from old classical screen-plays to freshly-minted academia.
Without her help, retrieving one of these items may get tricky, or nigh impossible.
Where would you look to get that one paper you really need?
Where can you get your hands on newly released digital page-turners
that aren't bogged down DRM to the point you need Abobe's permissions to read its content?
Do you really have to pay the Evil Conglomerates humongous sums for that *single* paper?

The Search might fear you, for there are many a trap, but fret not,
`bookwyrm` will do the dirty work for you.
Just give her a DOI, ISBN, title, URL or other entity used to identify reading material
and she'll fetch it for you - be it a childrens' adventure book or the Necronomicon.
No due time, guaranteed!

## How will `bookwyrm` work?
You give her some data which relates to a book, paper or similar.
With this data, she'll look trough all manners of sources (see [here](#planned-supported-sources)),
find some URIs, and then use an external downloader of your choice for processing.
By default, `bookwyrm` might use `aria2c`,
or perhaps something that all systems have at boot.
Not sure yet.

## Planned supported sources
### High priority
* Library Genesis
* Bookzz and Booksc
* Sci-Hub
* IRC channels
    - #ebooks on irc-highway
    - etc.
### Medium priority
* Various torrent sides
    - Private (bB, biblotik, etc.)
    - Torrent Project
    - etc.

