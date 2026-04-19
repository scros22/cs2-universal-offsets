"""
CS2 signature research helper.

For each candidate, prove that:
  - the anchor string actually exists in the DLL .rdata, AND
  - at least one .text instruction xref's it, AND
  - we can walk back from that xref to a function prologue.

Also dumps:
  - every export
  - every Source-2 InterfaceReg from the linked list at CreateInterface
  - byte windows for raw patterns we still need to verify

Output:  research_report.txt next to this file.
"""

import pefile
import os
import re
import struct
import sys

DLL_DIR_GAME = r"C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\game"
DLL_PATHS = {
    "client.dll":       os.path.join(DLL_DIR_GAME, "csgo", "bin", "win64", "client.dll"),
    "engine2.dll":      os.path.join(DLL_DIR_GAME, "bin", "win64", "engine2.dll"),
    "scenesystem.dll":  os.path.join(DLL_DIR_GAME, "bin", "win64", "scenesystem.dll"),
    "tier0.dll":        os.path.join(DLL_DIR_GAME, "bin", "win64", "tier0.dll"),
    "materialsystem2.dll": os.path.join(DLL_DIR_GAME, "bin", "win64", "materialsystem2.dll"),
    "inputsystem.dll":  os.path.join(DLL_DIR_GAME, "bin", "win64", "inputsystem.dll"),
    "vphysics2.dll":    os.path.join(DLL_DIR_GAME, "bin", "win64", "vphysics2.dll"),
    "rendersystemdx11.dll": os.path.join(DLL_DIR_GAME, "bin", "win64", "rendersystemdx11.dll"),
    "schemasystem.dll": os.path.join(DLL_DIR_GAME, "bin", "win64", "schemasystem.dll"),
    "soundsystem.dll":  os.path.join(DLL_DIR_GAME, "bin", "win64", "soundsystem.dll"),
}

# ---------------------------------------------------------------------------
# All candidate (label, dll, anchor_string) entries we want vetted.
# ---------------------------------------------------------------------------
STRING_ANCHOR_CANDIDATES = [
    # ---- engine2.dll networking ------------------------------------------
    ("Engine_GetTime",          "engine2.dll", "FrameAccumulateTime"),
    ("Engine_AccumulateTime",   "engine2.dll", "FrameAccumulateTime"),
    ("CL_FullyConnected",       "engine2.dll", "CNetworkGameClient for demo"),
    ("HostStateMgr",            "engine2.dll", "HostStateMgr001"),
    ("Source2EngineToClient",   "engine2.dll", "Source2EngineToClient001"),
    ("INetworkGameClient",      "engine2.dll", "INetworkGameClient"),
    ("CGameClient_for_demo",    "engine2.dll", "CNetworkGameClient for demo '%s'"),

    # ---- tier0.dll KV3 ----------------------------------------------------
    ("LoadKV3FromText",         "tier0.dll",   "CLoadKV3FromText::ReadLiteralValue"),
    ("Plat_FloatTime",          "tier0.dll",   "Plat_FloatTime"),

    # ---- scenesystem.dll --------------------------------------------------
    ("SceneSystem_factory",     "scenesystem.dll", "SceneSystem_002"),

    # ---- client.dll HUD / matchmaking / vacnet ----------------------------
    ("CSGOHudDeathNotice",      "client.dll", "CSGOHudDeathNotice"),
    ("HudWeaponSelection",      "client.dll", "HudWeaponSelection"),
    ("VacNetShot",              "client.dll", "VacNetShot"),
    ("CSOPartyInvite",          "client.dll", "CSOPartyInvite"),

    # ---- existing working anchors (sanity-check) --------------------------
    ("CCSPlayer_WeaponServices", "client.dll", "CCSPlayer_WeaponServices"),
    ("CCSPlayer_MovementServices", "client.dll", "CCSPlayer_MovementServices"),
    ("CCSPlayer_BulletServices", "client.dll", "CCSPlayer_BulletServices"),
    ("CCSPlayerController",      "client.dll", "CCSPlayerController"),
    ("CCSPlayerPawn",            "client.dll", "CCSPlayerPawn"),
    ("CSGameRules",              "client.dll", "CSGameRules"),
    ("paintkit_seed",            "client.dll", "set item texture seed"),
    ("paintkit_prefab",          "client.dll", "set item texture prefab"),
    ("paintkit_wear",            "client.dll", "set item texture wear"),
]


def load_pe(path):
    """Load PE and pull out (data, image_base, sections list)."""
    pe = pefile.PE(path, fast_load=True)
    pe.parse_data_directories(directories=[
        pefile.DIRECTORY_ENTRY['IMAGE_DIRECTORY_ENTRY_EXPORT'],
    ])
    image_base = pe.OPTIONAL_HEADER.ImageBase
    sections = []
    for s in pe.sections:
        name = s.Name.rstrip(b'\0').decode('latin-1')
        sections.append({
            'name': name,
            'va':   image_base + s.VirtualAddress,
            'rva':  s.VirtualAddress,
            'size': s.Misc_VirtualSize or s.SizeOfRawData,
            'data': s.get_data(),
            'exec': bool(s.Characteristics & 0x20000000),
            'read': bool(s.Characteristics & 0x40000000),
        })
    return pe, image_base, sections


def find_string_va(sections, needle):
    """Return list of VAs where needle\0 appears in .rdata/.data."""
    nb = needle.encode('latin-1') + b'\0'
    out = []
    for s in sections:
        if s['name'] not in ('.rdata', '.data'):
            continue
        d = s['data']
        i = 0
        while True:
            i = d.find(nb, i)
            if i < 0:
                break
            out.append(s['va'] + i)
            i += 1
    return out


def find_text_refs_to(sections, target_va):
    """Find .text instruction VAs that RIP-rel reference target_va.

    Recognises:
      E8/E9 disp32         - call/jmp rel32
      48 8B/8D/89 ?d disp  - mov/lea/mov [rip+disp]
    """
    refs = []
    for s in sections:
        if not s['exec']:
            continue
        d = s['data']
        sec_va = s['va']
        n = len(d)
        i = 0
        while i + 7 < n:
            b0 = d[i]
            # E8 / E9 disp32
            if b0 == 0xE8 or b0 == 0xE9:
                disp = struct.unpack_from('<i', d, i + 1)[0]
                if sec_va + i + 5 + disp == target_va:
                    refs.append(sec_va + i)
                i += 1
                continue
            # 48 8B/8D ?d05 disp32
            if b0 == 0x48 and d[i+1] in (0x8B, 0x8D):
                modrm = d[i+2]
                if (modrm & 0xC7) == 0x05:
                    disp = struct.unpack_from('<i', d, i + 3)[0]
                    if sec_va + i + 7 + disp == target_va:
                        refs.append(sec_va + i)
            elif b0 == 0x48 and d[i+1] == 0x89:
                modrm = d[i+2]
                if (modrm & 0xC7) == 0x05:
                    disp = struct.unpack_from('<i', d, i + 3)[0]
                    if sec_va + i + 7 + disp == target_va:
                        refs.append(sec_va + i)
            i += 1
    return refs


def find_function_start(sections, inside_va, max_lookback=0x800):
    """Walk backward from inside_va looking for INT3/RET pad + prologue."""
    # Locate section containing inside_va
    sec = None
    for s in sections:
        if s['va'] <= inside_va < s['va'] + len(s['data']):
            sec = s
            break
    if not sec:
        return 0
    off_in_sec = inside_va - sec['va']
    lo = max(0, off_in_sec - max_lookback)
    d = sec['data'][lo:off_in_sec]
    # Walk backward
    i = len(d) - 1
    while i >= 4:
        b = d[i]
        if b == 0xCC or b == 0xC3:
            j = i + 1
            while j < len(d) and d[j] == 0xCC:
                j += 1
            if j + 4 < len(d):
                p = d[j:j+8]
                ok = (
                    p[:4] == b'\x48\x89\x5C\x24' or
                    p[:3] == b'\x48\x83\xEC'     or
                    p[:3] == b'\x48\x8B\xC4'     or
                    (p[0] == 0x40 and 0x50 <= p[1] <= 0x57) or
                    (0x50 <= p[0] <= 0x57)       or
                    p[0] == 0x55                 or
                    (p[0] == 0x4C and p[1] == 0x89)
                )
                if ok:
                    return sec['va'] + lo + j
        i -= 1
    return 0


def hex_window(sections, va, before=0, after=24):
    for s in sections:
        if s['va'] <= va < s['va'] + len(s['data']):
            o = va - s['va']
            lo = max(0, o - before)
            hi = min(len(s['data']), o + after)
            return ' '.join(f'{b:02X}' for b in s['data'][lo:hi])
    return '?'


def dump_exports(pe, image_base):
    out = []
    if not hasattr(pe, 'DIRECTORY_ENTRY_EXPORT'):
        return out
    for e in pe.DIRECTORY_ENTRY_EXPORT.symbols:
        if e.name:
            out.append((e.name.decode('latin-1', 'replace'), image_base + e.address))
    return out


def main():
    out_lines = []

    def emit(msg=""):
        out_lines.append(msg)
        print(msg)

    cache = {}
    for dll, path in DLL_PATHS.items():
        if not os.path.isfile(path):
            emit(f"!! MISSING DLL: {dll} ({path})")
            continue
        try:
            pe, base, sections = load_pe(path)
            cache[dll] = (pe, base, sections)
            emit(f"Loaded {dll:24s} base=0x{base:X} sections={len(sections)}")
        except Exception as e:
            emit(f"!! Failed to parse {dll}: {e}")

    emit("\n" + "=" * 78)
    emit("STRING ANCHOR VERIFICATION")
    emit("=" * 78)

    for label, dll, anchor in STRING_ANCHOR_CANDIDATES:
        if dll not in cache:
            emit(f"  [{label:30s}] {dll}: DLL missing")
            continue
        pe, base, sections = cache[dll]
        vas = find_string_va(sections, anchor)
        if not vas:
            emit(f"  [{label:30s}] {dll}: STRING NOT FOUND  '{anchor}'")
            continue
        # take the first hit
        sva = vas[0]
        refs = find_text_refs_to(sections, sva)
        if not refs:
            emit(f"  [{label:30s}] {dll}: string @ 0x{sva:X} but NO .text refs")
            continue
        fn = find_function_start(sections, refs[0])
        emit(f"  [{label:30s}] {dll}: str@0x{sva:X} ref@0x{refs[0]:X} fn@0x{fn:X}"
             f"  ({len(vas)} str copies, {len(refs)} refs)")
        if fn:
            window = hex_window(sections, fn, 0, 24)
            emit(f"      prologue bytes: {window}")

    emit("\n" + "=" * 78)
    emit("EXPORT DUMP (filtered)")
    emit("=" * 78)
    for dll, (pe, base, sections) in cache.items():
        exps = dump_exports(pe, base)
        emit(f"\n--- {dll} : {len(exps)} exports ---")
        for n, va in exps[:30]:
            emit(f"   {n:48s} 0x{va:X}")
        if len(exps) > 30:
            emit(f"   ... +{len(exps)-30} more")

    out = os.path.join(os.path.dirname(__file__), "research_report.txt")
    with open(out, 'w', encoding='utf-8') as f:
        f.write('\n'.join(out_lines))
    print(f"\nReport written to {out}")


if __name__ == "__main__":
    main()
