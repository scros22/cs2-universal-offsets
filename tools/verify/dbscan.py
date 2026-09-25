"""Offline rescan of the dumper's pattern database against the CS2 DLLs on disk,
mirroring src/patterns/mod.rs (.text all-hits first, .rdata first-hit, full image
last; Rel32/RipRel resolve with extra_off). Compares each resolved target's opening
bytes with the previous dump's captured prologue (include/patterns/patterns.json)
to flag silent drift. No game process needed.

    py tools/verify/dbscan.py            # prints a summary, writes dbscan.json next to this file
    set CS2_GAME_DIR=...                 # override the game directory
"""
import io, os, re, json, struct, sys

REPO = os.environ.get('CS2_SDK_REPO') or os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
GAME = os.environ.get('CS2_GAME_DIR') or r'C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\game'
CSGO_MODS = {'client.dll', 'server.dll', 'matchmaking.dll'}

def mod_path(m):
    return os.path.join(GAME, 'csgo' if m in CSGO_MODS else '', 'bin', 'win64', m) if m in CSGO_MODS \
        else os.path.join(GAME, 'bin', 'win64', m)

class Img:
    """PE mapped as in memory (sections at their VA)."""
    def __init__(self, path):
        raw = open(path, 'rb').read()
        e = struct.unpack_from('<I', raw, 0x3C)[0]
        nsec = struct.unpack_from('<H', raw, e + 6)[0]
        opt = struct.unpack_from('<H', raw, e + 20)[0]
        self.base = struct.unpack_from('<Q', raw, e + 24 + 24)[0]
        size = struct.unpack_from('<I', raw, e + 24 + 56)[0]
        hdr = struct.unpack_from('<I', raw, e + 24 + 60)[0]
        img = bytearray(size)
        img[:hdr] = raw[:hdr]
        self.secs = {}
        so = e + 24 + opt
        for k in range(nsec):
            name = raw[so:so + 8].rstrip(b'\0').decode(errors='replace')
            vsz, va, rsz, rp = struct.unpack_from('<IIII', raw, so + 8)
            n = min(rsz, vsz if vsz else rsz)
            img[va:va + n] = raw[rp:rp + n]
            self.secs[name] = (va, vsz)
            so += 40
        self.img = bytes(img)

def parse_db():
    src = io.open(os.path.join(REPO, 'src', 'patterns', 'database.rs'), encoding='utf-8-sig').read()
    out = []
    for m in re.finditer(r'Pattern\s*\{(.*?prototype\s*:\s*"(?:[^"\\]|\\.)*"\s*,?\s*)\}', src, re.S):
        body = m.group(1)
        g = lambda k: (re.search(k + r'\s*:\s*"((?:[^"\\]|\\.)*)"', body) or [None, None])[1]
        mr = re.search(r'resolve\s*:\s*ResolveKind::(\w+)\s*\{\s*rel_off\s*:\s*(\d+)', body)
        if mr:
            rs = ('REL' if mr.group(1) == 'Rel32' else 'RIP') + ':' + mr.group(2)
        else:
            rs = re.search(r'resolve\s*:\s*(\w+)', body).group(1)
        eo = int(re.search(r'extra_off\s*:\s*(-?(?:0x[0-9A-Fa-f]+|\d+))', body).group(1), 0)
        out.append({'name': g('name'), 'module': g('module'), 'needle': g('needle'), 'resolve': rs, 'extra_off': eo,
                    'line': src[:m.start()].count('\n') + 1})
    return out

def compile_needle(n):
    toks = n.split()
    return re.compile(b''.join(b'.' if t in ('?', '??') else re.escape(bytes([int(t, 16)])) for t in toks), re.S)

def s32(v): return v - (1 << 32) if v & 0x80000000 else v

def scan(img, e):
    rx = compile_needle(e['needle'])
    tva, tsz = img.secs.get('.text', (0, 0))
    text = img.img[tva:tva + tsz]
    hits = [tva + m.start() for m in rx.finditer(text)]
    n = len(hits)
    off = hits[0] if hits else None
    if off is None and '.rdata' in img.secs:
        rva, rsz = img.secs['.rdata']
        m = rx.search(img.img, rva, rva + rsz)
        if m: off, n = m.start(), 1
    if off is None:
        m = rx.search(img.img)
        if m: off, n = m.start(), 1
    if off is None:
        return None, 0
    if e['resolve'] == 'NONE':
        return off + e['extra_off'], n   # fixed dumper: extra_off moves rva too
    rel = {'REL32_1': 1, 'RIPREL_3': 3, 'RIPREL_2': 2}.get(e['resolve']) or int(e['resolve'].split(':')[1])
    disp = s32(struct.unpack_from('<I', img.img, off + rel)[0])
    return off + rel + 4 + disp + e['extra_off'], n

def relocatable_ok(old_hex, new_bytes):
    """Loose prologue compare: equal length prefix ignoring bytes that differ inside
    likely disp/imm fields. Returns fraction of matching bytes."""
    ob = bytes.fromhex(old_hex.replace(' ', ''))
    nb = new_bytes[:len(ob)]
    if not ob: return None
    same = sum(1 for a, b in zip(ob, nb) if a == b)
    return same / len(ob)

def main():
    db = parse_db()
    prev = {}
    t = io.open(os.path.join(REPO, 'include', 'patterns', 'patterns.json'), encoding='utf-8-sig').read()
    t = re.sub(r'(?<=[:\s,\[])0x([0-9A-Fa-f]+)', lambda m: str(int(m.group(1), 16)), t)
    for p in json.loads(t)['patterns']:
        prev[(p['module'], p['pattern'])] = p
    imgs = {}
    rows = []
    for e in db:
        m = e['module']
        if m not in imgs:
            imgs[m] = Img(mod_path(m))
        img = imgs[m]
        rva, n = scan(img, e)
        p = prev.get((m, e['needle']))
        row = dict(e, rva=rva, hits=n, prev_rva=p['rva'] if p else None, prev_bytes=p.get('bytes') if p else None)
        if rva is not None and p and p.get('bytes'):
            row['prologue_match'] = relocatable_ok(p['bytes'], img.img[rva:rva + 24])
        rows.append(row)
    json.dump(rows, open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'dbscan.json'), 'w'), indent=1)
    miss = [r for r in rows if r['rva'] is None]
    multi = [r for r in rows if r['hits'] > 1]
    drift = [r for r in rows if r['rva'] is not None and r.get('prologue_match') is not None and r['prologue_match'] < 0.75]
    noprev = [r for r in rows if r['prev_rva'] is None]
    print('total %d | missing %d | multi-hit %d | prologue<75%% %d | not in previous dump %d' % (len(rows), len(miss), len(multi), len(drift), len(noprev)))
    from collections import Counter
    print('missing by module:', dict(Counter(r['module'] for r in miss)))
    print('drift by module:', dict(Counter(r['module'] for r in drift)))
    print('multi by module:', dict(Counter(r['module'] for r in multi)))

if __name__ == "__main__": main()
