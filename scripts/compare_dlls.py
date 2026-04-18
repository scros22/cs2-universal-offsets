import sys

def get_strings(path, min_len=6):
    data = open(path, 'rb').read()
    strs = set()
    cur = []
    for b in data:
        if 0x20 <= b < 0x7f:
            cur.append(chr(b))
        else:
            if len(cur) >= min_len:
                strs.add(''.join(cur))
            cur = []
    return strs

w = get_strings('x64/CS2_working.dll')
c = get_strings('x64/Release/CS2.dll')
only_current = sorted(c - w)
only_working = sorted(w - c)
print(f'Strings only in CURRENT BUILD ({len(only_current)}):')
for s in only_current[:60]:
    print(f'  + {s}')
print(f'\nStrings only in WORKING ({len(only_working)}):')
for s in only_working[:60]:
    print(f'  - {s}')
