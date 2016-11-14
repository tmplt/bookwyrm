import bencodepy
import hashlib
import base64
import requests
from furl import furl

SI_PREFIXES = {
    'k': 1e3,  # kilo
    'M': 1e6,  # Mega
    'G': 1e9   # Giga
}

ORDINAL_NUMS = {
    1: 'st',
    2: 'nd',
    3: 'rd'
    # 'th' in ordinal_num()
}


def magnet_from_torrent(torrent):
    """
    Generate a torrent magnet link from a .torrent-file.
    """

    metadata = bencodepy.decode(torrent)
    subj = metadata[b'info']
    hashcontents = bencodepy.encode(subj)
    digest = hashlib.sha1(hashcontents).digest()
    b32hash = base64.b32encode(digest).decode()

    return 'magnet:?' \
        + 'xt=urn:btih:' + b32hash \
        + '&dn=' + metadata[b'info'][b'name'].decode() \
        + '&tr=' + metadata[b'announce'].decode() \
        + '&xl=' + str(metadata[b'info'][b'length'])


def http_domain(url):
    """
    Return the top-level domain from an URL;
    e.g. "libgen.io" from "http://libgen.io/?req=temeraire".
    """

    f = furl(url)
    return f.host


def download(uri, filename=None, referrer=None):
    """
    HTTP GET the file at `uri` as `filename`, with
    the referer header `referrer`.
    """

    if uri.startswith("http"):

        if referrer is not None:
            headers = {'referer': referrer}
        else:
            headers = {'referer': http_domain(uri)}

        r = requests.get(uri, headers=headers)

        # check filename here

        with open(filename, 'w') as f:
            for chunk in r.iter_content(chunk_size=1024):
                if chunk:
                    f.write(chunk)

    elif uri.startswith("magnet:?"):
        pass


def ordinal_num(n):
    """
    Concatenate the integer `n` with its ordinal
    "st", "nd", "rd" or "th".
    """

    return "%d%s" % (
        n,
        ORDINAL_NUMS.get(n if n < 20 else n % 10, 'th')
    )


def translate_si_prefix(key):
    return SI_PREFIXES.get(key, None)


def valid_doi(doi):
    """
    A very simple validation:
    only checks whether the suffix contains numbers.
    Is is possible to validate the suffix?
    """

    prefix, suffix = doi.split('/', 1)
    for part in prefix:
        if not part.isdigit():
            return False

    return True


def write(content):
    with open(content['name'], 'bw+') as f:
        f.write(content['pdf'])
