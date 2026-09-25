"""Copy the patterns the dumper re-anchored on its own into the database.

When a database pattern stops matching after a CS2 update but the function did
not change, the dumper re-finds it through the previous dump's prologue bytes,
generates a fresh unique pattern and publishes it in patterns.json with a
`healed_from` field (see src/patterns/mod.rs::heal). This script writes those
new patterns into src/patterns/database.rs so the next build of the dumper
matches directly again.

    py tools/verify/heal.py                 # report what would change
    py tools/verify/heal.py --apply         # rewrite database.rs
    py tools/verify/heal.py --dump <dir>    # a dump directory other than include/
"""
import io, os, re, sys, json

REPO = os.environ.get('CS2_SDK_REPO') or os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
ENTRY = re.compile(r'[ \t]*Pattern\s*\{.*?prototype\s*:\s*"(?:[^"\\]|\\.)*"\s*,?\s*\}\s*,\n', re.S)


def display_name(raw):
    """Python twin of src/patterns/mod.rs::display_name (published names)."""
    if not raw:
        return raw
    if '::' in raw:
        return raw.rsplit('::', 1)[1]
    if raw.startswith(('m_', 'dw', 'g_', 'C_')) or raw.endswith('_t'):
        return raw
    parts = [p for p in raw.split('_') if p]
    if len(parts) > 1:
        head = parts[0]
        if len(head) >= 2 and head[0] in 'CI' and head[1].isupper():
            rest = '_'.join(parts[1:])
            multi = any(c.isupper() or c == '_' for c in rest[1:])
            if rest[:1].isupper() and multi:
                return rest
    return raw


def healed_hits(dump_dir):
    p = os.path.join(dump_dir, 'patterns', 'patterns.json')
    d = json.load(io.open(p, encoding='utf-8-sig'))
    return [h for h in d['patterns'] if h.get('healed_from')]


def main():
    apply = '--apply' in sys.argv
    dump = sys.argv[sys.argv.index('--dump') + 1] if '--dump' in sys.argv else os.path.join(REPO, 'include')
    healed = healed_hits(dump)
    if not healed:
        print('nothing to do: no healed_from entries in', dump)
        return
    p = os.path.join(REPO, 'src', 'patterns', 'database.rs')
    raw = io.open(p, 'rb').read()
    bom = raw.startswith(b'\xef\xbb\xbf')
    s = raw.decode('utf-8-sig')
    nl = '\r\n' if '\r\n' in s else '\n'
    s = s.replace('\r\n', '\n')
    by_pub = {h['name']: h for h in healed}
    changed = []

    def fix(m):
        e = m.group(0)
        nm = re.search(r'name\s*:\s*"([^"]+)"', e).group(1)
        mod = re.search(r'module\s*:\s*"([^"]+)"', e).group(1)
        h = by_pub.get(display_name(nm)) or by_pub.get(nm)
        if not h or h['module'].lower() != mod.lower():
            return e
        old = re.search(r'needle\s*:\s*"([^"]*)"', e).group(1)
        if old != h['healed_from']:
            return e
        e = re.sub(r'needle\s*:\s*"[^"]*"', lambda _m: 'needle: "%s"' % h['pattern'], e)
        e = re.sub(r'resolve\s*:\s*[A-Za-z0-9_:{}\s]+?,', 'resolve: NONE,', e, count=1)
        e = re.sub(r'extra_off\s*:\s*-?(?:0x[0-9A-Fa-f]+|\d+)', 'extra_off: 0', e)
        changed.append({'name': nm, 'module': mod, 'rva': h['rva'], 'old': old, 'new': h['pattern']})
        return e

    s2 = ENTRY.sub(fix, s)
    for c in changed:
        print('%s %s (%s): %s\n    -> %s' % (c['module'], c['name'], c['rva'], c['old'], c['new']))
    missing = [h['name'] for h in healed if h['name'] not in {display_name(c['name']) for c in changed} and h['name'] not in {c['name'] for c in changed}]
    if missing:
        print('not matched to a database entry (old pattern differs?):', missing)
    if apply and changed:
        io.open(p, 'w', encoding='utf-8-sig' if bom else 'utf-8', newline='').write(s2.replace('\n', nl))
        print('database.rs updated: %d entries. Rebuild the dumper.' % len(changed))
    elif changed:
        print('%d entries would change; re-run with --apply.' % len(changed))


if __name__ == '__main__':
    main()
