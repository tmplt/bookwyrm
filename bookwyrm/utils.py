import bencodepy
import hashlib
import base64

# Credit to Danilo "DanySK" Pianini
# <https://github.com/DanySK/torrent2magnet>
def magnet_from_torrent(torrent):
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
    return url.split('/')[2]

def download(uri, filename=None, referrer=None):
    if uri.startswith("http"):
        s = requests.Session()

        if referrer is not None:
            s.headers.update({'referer': referrer})

        r = s.get(uri)

        # check filename here

        with open(filename, 'w') as f:
            for chunk in r.iter_content(chunk_size=1024):
                if chunk:
                    f.write(chunk)

    elif uri.startswith("magnet:?"):
        pass

def ordinal_num(n):
    ordinals = {
        1: 'st',
        2: 'nd',
        3: 'rd'
    }

    return "%d%s" % (
        n,
        ordinals.get(n if n < 20 else n % 10, 'th')
    )
