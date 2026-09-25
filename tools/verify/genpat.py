"""Deterministic IDA-style pattern generator + verifier for the dumper DB.

  gen_func(module, rva)   -> needle that matches ONCE in .text, starting AT rva (resolve NONE)
  gen_global(module, rva) -> (needle, rel_off) for a rip-relative reference to a global (RipRel)
  verify(module, needle, resolve, extra_off) -> (resolved_rva, text_hits)  (dumper semantics)

Wildcards: branch/call rel operands, RIP disp32, memory disp32, and imm32 (struct offsets,
frame sizes, constants that drift between builds). Short (8-bit) displacements are kept."""
import os, re, struct, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from dbscan import Img, mod_path, compile_needle, s32
import capstone
from capstone import x86

_CS = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_64)
_CS.detail = True
_IMG = {}

def img(m):
    if m not in _IMG:
        _IMG[m] = Img(mod_path(m))
    return _IMG[m]

def _text(im):
    va, sz = im.secs['.text']
    return va, im.img[va:va + sz]

def _mask_insn(ins):
    """Return list of (byte or None) for one instruction."""
    b = list(ins.bytes)
    wild = set()
    co = ins.disp_offset if hasattr(ins, 'disp_offset') else 0
    # capstone gives encoding offsets for disp / imm
    try:
        enc_disp_off, enc_disp_size = ins.disp_offset, ins.disp_size
        enc_imm_off, enc_imm_size = ins.imm_offset, ins.imm_size
    except AttributeError:
        enc_disp_off = enc_disp_size = enc_imm_off = enc_imm_size = 0
    is_branch = ins.group(capstone.CS_GRP_JUMP) or ins.group(capstone.CS_GRP_CALL)
    for op in ins.operands:
        if op.type == x86.X86_OP_MEM:
            if op.mem.base == x86.X86_REG_RIP or enc_disp_size >= 4:
                for k in range(enc_disp_off, enc_disp_off + enc_disp_size): wild.add(k)
        elif op.type == x86.X86_OP_IMM:
            if is_branch or enc_imm_size >= 4:
                for k in range(enc_imm_off, enc_imm_off + enc_imm_size): wild.add(k)
    return [None if i in wild else v for i, v in enumerate(b)]

def _fmt(toks):
    s = ' '.join('?' if t is None else '%02X' % t for t in toks)
    return re.sub(r'(\s\?)+$', '', s)

def count_text(m, needle):
    im = img(m)
    tva, text = _text(im)
    return [tva + x.start() for x in compile_needle(needle).finditer(text)]

def gen_from(m, start, min_len=10, max_len=96, max_insns=40):
    """Grow a needle from `start` instruction by instruction until unique in .text."""
    im = img(m)
    toks = []
    cur = start
    for ins in _CS.disasm(im.img[start:start + 256], start):
        toks += _mask_insn(ins)
        cur = ins.address + ins.size
        fixed = sum(1 for t in toks if t is not None)
        if len(toks) >= min_len and fixed >= 6:
            nd = _fmt(toks)
            hits = count_text(m, nd)
            if len(hits) == 1 and hits[0] == start:
                return nd
        if len(toks) >= max_len or ins.mnemonic in ('ret', 'int3', 'jmp') and len(toks) > max_len // 2:
            break
    return None

def gen_func(m, rva):
    return gen_from(m, rva)

def gen_global(m, target):
    """Find rip-relative references to `target`, return the shortest unique (needle, rel_off, site)."""
    im = img(m)
    tva, text = _text(im)
    best = None
    # candidate sites: any 4 bytes that equal target - (site_end)
    for mo in re.finditer(b'(?s).', b''):  # placeholder to keep structure simple
        pass
    cands = []
    for i in range(0, len(text) - 8):
        # quick filter on common rip-relative opcodes: 48 8B/8D/89 05|0D|15|1D|25|2D|35|3D, 4C .., 8B 05, FF 15 etc.
        pass
    # Linear decode is too slow for 20+ MB; scan raw disp32 candidates instead.
    for k in range(3, 8):          # disp32 sits 2..7 bytes into the instruction
        pass
    for off in range(len(text) - 4):
        pass
    return best

def gen_global_fast(m, target, max_sites=8):
    """Vectorised search: disp32 at position p refers to target if p+4+disp == target (for disp at insn end)
    or p+4+imm_size+disp == target (lea/mov with trailing imm). We scan for the 4-byte value
    (target - (tva+p+4)) at every p using a rolling compare on candidate opcode prefixes."""
    im = img(m)
    tva, text = _text(im)
    sites = []
    # common prefixes of rip-relative insns (the disp follows these bytes directly)
    prefixes = [b'\x48\x8b', b'\x48\x8d', b'\x48\x89', b'\x4c\x8b', b'\x4c\x8d', b'\x4c\x89', b'\x8b', b'\x89',
                b'\x48\x39', b'\x4c\x39', b'\x48\x3b', b'\x48\x83\x3d', b'\x83\x3d', b'\xff\x15', b'\xff\x05', b'\x48\xc7\x05',
                b'\xf3\x0f\x10', b'\xf3\x0f\x11', b'\xf2\x0f\x10', b'\x0f\x10', b'\x0f\x11', b'\x80\x3d', b'\xc6\x05', b'\x88',
                b'\x0f\xb6', b'\x0f\xb7', b'\x39', b'\x3b', b'\x48\x63', b'\xc7\x05', b'\x66\x0f\x6e', b'\x44\x8b', b'\x44\x89']
    for mo in re.finditer(rb'(?s)(?:[\x48\x4c\x44\x66\xf2\xf3]?[\x0f]?[\x8b\x8d\x89\x39\x3b\x10\x11\x63\xb6\xb7\x6e\x83\x80\xc6\xc7\x88\xff])([\x05\x0d\x15\x1d\x25\x2d\x35\x3d])', text):
        p = mo.end()  # disp starts here
        if p + 4 > len(text): continue
        disp = s32(struct.unpack_from('<I', text, p)[0])
        # cheap filter: instruction end is disp_end + trailing imm (0/1/2/4 bytes)
        if target - disp - (tva + p + 4) not in (0, 1, 2, 4):
            continue
        # figure trailing immediate size by decoding the instruction
        ins_start = mo.start()
        for ins in _CS.disasm(text[ins_start:ins_start + 15], tva + ins_start):
            if ins.address != tva + ins_start: break
            end = ins.address + ins.size
            for op in ins.operands:
                if op.type == x86.X86_OP_MEM and op.mem.base == x86.X86_REG_RIP:
                    if end + op.mem.disp == target:
                        sites.append((tva + ins_start, ins.disp_offset))
            break
        if len(sites) >= max_sites: break
    out = []
    for site, doff in sites:
        nd = gen_from(m, site, min_len=8)
        if nd:
            out.append((len(nd), nd, doff, site))
    out.sort()
    return out[0][1:] if out else None

def gen_call(m, target, max_sites=16):
    """Unique needle starting at a CALL/JMP rel32 whose target is `target` (resolve REL32_1)."""
    im = img(m)
    tva, text = _text(im)
    sites = []
    for mo in re.finditer(rb'(?s)[\xe8\xe9]', text):
        p = mo.start()
        if p + 5 > len(text): break
        disp = s32(struct.unpack_from('<I', text, p + 1)[0])
        if tva + p + 5 + disp == target:
            sites.append(tva + p)
            if len(sites) >= max_sites: break
    out = []
    for site in sites:
        nd = gen_from(m, site, min_len=10)
        if nd:
            out.append((len(nd), nd, site))
    out.sort()
    return (out[0][1], out[0][2]) if out else None

def verify(m, needle, resolve='NONE', extra_off=0):
    im = img(m)
    tva, text = _text(im)
    hits = [tva + x.start() for x in compile_needle(needle).finditer(text)]
    if not hits: return None, 0
    off = hits[0]
    if resolve == 'NONE':
        return off + extra_off, len(hits)
    rel = {'REL32_1': 1, 'RIPREL_3': 3, 'RIPREL_2': 2}.get(resolve) or int(str(resolve).split(':')[1])
    disp = s32(struct.unpack_from('<I', im.img, off + rel)[0])
    return off + rel + 4 + disp + extra_off, len(hits)

if __name__ == '__main__':
    m, rva = sys.argv[1], int(sys.argv[2], 16)
    base = img(m).base
    if rva >= base: rva -= base
    if len(sys.argv) > 3 and sys.argv[3] == 'global':
        r = gen_global_fast(m, rva)
        print(r)
        if r: print(verify(m, r[0], 'RIP:%d' % r[1]))
    else:
        nd = gen_func(m, rva)
        print(nd)
        if nd: print(hex(verify(m, nd)[0]), verify(m, nd)[1])
