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
