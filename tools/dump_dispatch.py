"""Dump start of IRP_MJ_DEVICE_CONTROL handler at RVA 0x11984"""
import struct
from capstone import Cs, CS_ARCH_X86, CS_MODE_64

data = open(r"c:\Users\Samuel\License-Loader\Loader\Products\CS2\SIVX64.sys", "rb").read()
e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
ib = struct.unpack_from("<Q", data, e_lfanew+4+20+24)[0]
ns = struct.unpack_from("<H", data, e_lfanew+4+2)[0]
ohs = struct.unpack_from("<H", data, e_lfanew+4+16)[0]
ss = e_lfanew+4+20+ohs
secs = []
for i in range(ns):
    o=ss+i*40
    secs.append((struct.unpack_from("<I",data,o+12)[0], struct.unpack_from("<I",data,o+16)[0], struct.unpack_from("<I",data,o+20)[0]))

def r2o(rva):
    for vrva,rs,rp in secs:
        if vrva<=rva<vrva+rs: return rva-vrva+rp
    return None

imports = {}
off = r2o(struct.unpack_from("<I", data, e_lfanew+4+20+120)[0])
while off:
    ilt,nrva,iat = struct.unpack_from("<I",data,off)[0], struct.unpack_from("<I",data,off+12)[0], struct.unpack_from("<I",data,off+16)[0]
    if ilt==0 and nrva==0: break
    no = r2o(nrva)
    dn = data[no:data.index(b'\x00',no)].decode('ascii',errors='replace') if no else '?'
    io = r2o(ilt)
    idx=0
    if io:
        while True:
            e = struct.unpack_from("<Q",data,io+idx*8)[0]
            if e==0: break
            if e&(1<<63): fn=f"Ord#{e&0xFFFF}"
            else:
                ho=r2o(e&0x7FFFFFFF)
                fn=data[ho+2:data.index(b'\x00',ho+2)].decode('ascii',errors='replace') if ho else '?'
            imports[iat+idx*8]=f"{dn}!{fn}"
            idx+=1
    off+=20

md = Cs(CS_ARCH_X86, CS_MODE_64)
fo = r2o(0x11984)
code = data[fo:fo+0x1500]  # ~5KB
base = ib + 0x11984
n = 0
for insn in md.disasm(code, base):
    imp = None
    if insn.mnemonic in ('call','jmp') and 'qword ptr [rip' in insn.op_str and len(insn.bytes)>=6:
        d = struct.unpack_from("<i", bytes(insn.bytes), len(insn.bytes)-4)[0]
        trva = insn.address+insn.size+d-ib
        imp = imports.get(trva)
    mk = f"  ; {imp}" if imp else ""
    if insn.mnemonic=='lea' and 'rip' in insn.op_str:
        d = struct.unpack_from("<i", bytes(insn.bytes), len(insn.bytes)-4)[0]
        mk += f"  ; VA {insn.address+insn.size+d:#x}"
    if insn.mnemonic=='call' and insn.op_str.startswith('0x'):
        mk += f"  ; sub_{insn.op_str}"
    # Mark significant items
    if insn.mnemonic == 'cmp':
        for val in [4,8,0xc,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x30,0x34,0x38,0x3c,0x40]:
            if f", {val:#x}" in insn.op_str or f", {val}" in insn.op_str:
                mk += f"  ; *** CMD {val:#x}? ***"
    if '+ 0x18]' in insn.op_str: mk += " ; IoControlCode?"
    if '+ 0xb8]' in insn.op_str: mk += " ; CurrentStackLocation"
    
    print(f"{insn.address:#018x}: {insn.mnemonic:8s} {insn.op_str}{mk}")
    n += 1
    if n >= 400: break
