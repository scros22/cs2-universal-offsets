"""
SIVX64.sys targeted RE — find IRP_MJ_DEVICE_CONTROL and Cmd 0x14 handler
"""
import struct
from capstone import Cs, CS_ARCH_X86, CS_MODE_64

DRIVER_PATH = r"c:\Users\Samuel\License-Loader\Loader\Products\CS2\SIVX64.sys"

def read_driver():
    with open(DRIVER_PATH, "rb") as f:
        return f.read()

def parse_pe(data):
    e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
    coff = e_lfanew + 4
    num_sections = struct.unpack_from("<H", data, coff + 2)[0]
    opt_hdr_size = struct.unpack_from("<H", data, coff + 16)[0]
    opt = coff + 20
    entry_rva = struct.unpack_from("<I", data, opt + 16)[0]
    image_base = struct.unpack_from("<Q", data, opt + 24)[0]
    
    sections = []
    sec_start = opt + opt_hdr_size
    for i in range(num_sections):
        off = sec_start + i * 40
        name = data[off:off+8].rstrip(b'\x00').decode('ascii', errors='replace')
        vsize = struct.unpack_from("<I", data, off + 8)[0]
        vrva = struct.unpack_from("<I", data, off + 12)[0]
        raw_size = struct.unpack_from("<I", data, off + 16)[0]
        raw_ptr = struct.unpack_from("<I", data, off + 20)[0]
        sections.append({'name': name, 'vrva': vrva, 'vsize': vsize,
                        'raw_ptr': raw_ptr, 'raw_size': raw_size})
    
    import_rva = struct.unpack_from("<I", data, opt + 120)[0]
    return {'image_base': image_base, 'entry_rva': entry_rva,
            'sections': sections, 'import_rva': import_rva}

def rva_to_offset(sections, rva):
    for s in sections:
        if s['vrva'] <= rva < s['vrva'] + s['raw_size']:
            return rva - s['vrva'] + s['raw_ptr']
    return None

def offset_to_rva(sections, foffset):
    for s in sections:
        if s['raw_ptr'] <= foffset < s['raw_ptr'] + s['raw_size']:
            return foffset - s['raw_ptr'] + s['vrva']
    return None

def parse_imports(data, pe):
    imports = {}
    off = rva_to_offset(pe['sections'], pe['import_rva'])
    if off is None:
        return imports
    while True:
        ilt_rva = struct.unpack_from("<I", data, off)[0]
        name_rva = struct.unpack_from("<I", data, off + 12)[0]
        iat_rva = struct.unpack_from("<I", data, off + 16)[0]
        if ilt_rva == 0 and name_rva == 0:
            break
        name_off = rva_to_offset(pe['sections'], name_rva)
        dll_name = data[name_off:data.index(b'\x00', name_off)].decode('ascii', errors='replace') if name_off else "?"
        ilt_off = rva_to_offset(pe['sections'], ilt_rva)
        idx = 0
        if ilt_off:
            while True:
                entry = struct.unpack_from("<Q", data, ilt_off + idx * 8)[0]
                if entry == 0: break
                if entry & (1 << 63):
                    func_name = f"Ord#{entry & 0xFFFF}"
                else:
                    hint_off = rva_to_offset(pe['sections'], entry & 0x7FFFFFFF)
                    func_name = data[hint_off+2:data.index(b'\x00', hint_off+2)].decode('ascii', errors='replace') if hint_off else "?"
                imports[iat_rva + idx * 8] = f"{dll_name}!{func_name}"
                idx += 1
        off += 20
    return imports

def get_iat_va(pe, iat_rva):
    return pe['image_base'] + iat_rva

def resolve_import_call(insn_bytes, insn_addr, insn_size, pe, imports):
    """Resolve a RIP-relative call/jmp to import name"""
    if len(insn_bytes) >= 6:
        disp = struct.unpack_from("<i", bytes(insn_bytes), len(insn_bytes) - 4)[0]
        target_va = insn_addr + insn_size + disp
        target_rva = target_va - pe['image_base']
        if target_rva in imports:
            return imports[target_rva]
    return None

def disasm_at(data, pe, rva, size, imports):
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True
    foff = rva_to_offset(pe['sections'], rva)
    if foff is None: return []
    code = data[foff:foff+size]
    base = pe['image_base'] + rva
    result = []
    for insn in md.disasm(code, base):
        imp = None
        if insn.mnemonic in ('call', 'jmp') and 'qword ptr [rip' in insn.op_str:
            imp = resolve_import_call(insn.bytes, insn.address, insn.size, pe, imports)
        result.append({
            'addr': insn.address, 'rva': insn.address - pe['image_base'],
            'mnemonic': insn.mnemonic, 'op_str': insn.op_str,
            'bytes': bytes(insn.bytes), 'size': insn.size,
            'import': imp
        })
    return result

def main():
    data = read_driver()
    pe = parse_pe(data)
    imports = parse_imports(data, pe)
    
    print(f"Image base: {pe['image_base']:#x}")
    print(f"Entry RVA: {pe['entry_rva']:#x}")
    print(f"\n=== ALL IMPORTS ===")
    for rva, name in sorted(imports.items()):
        print(f"  IAT RVA {rva:#06x} (VA {pe['image_base']+rva:#x}): {name}")
    
    # Search for MmMapIoSpace, MmCopyMemory strings (dynamically resolved)
    print(f"\n=== DYNAMIC API RESOLUTION STRINGS ===")
    for func in ['MmMapIoSpace', 'MmCopyMemory', 'MmGetPhysicalAddress', 'MmUnmapIoSpace',
                 'MmGetSystemRoutineAddress', 'ZwMapViewOfSection', 'MmAllocateContiguousMemory',
                 'MmMapLockedPagesSpecifyCache', 'MmProbeAndLockPages', 'IoAllocateMdl',
                 'MmBuildMdlForNonPagedPool', 'MmMapLockedPages']:
        for needle in [func.encode('ascii'), func.encode('utf-16-le')]:
            idx = data.find(needle)
            while idx != -1:
                rva = offset_to_rva(pe['sections'], idx)
                enc = 'unicode' if len(needle) > len(func) else 'ascii'
                print(f"  '{func}' ({enc}) at file offset {idx:#x}, RVA {rva:#x}" if rva else f"  '{func}' ({enc}) at offset {idx:#x}")
                idx = data.find(needle, idx + 1)
    
    # === Find IRP_MJ_DEVICE_CONTROL ===
    # In DriverEntry, the driver sets MajorFunction[IRP_MJ_DEVICE_CONTROL] (offset 0xE0)
    # Pattern: mov qword ptr [reg + 0xE0], some_addr
    # Or: lea reg2, [rip + disp]; mov [reg + 0xE0], reg2
    
    print(f"\n=== DRIVERENTRY ANALYSIS ===")
    entry = disasm_at(data, pe, pe['entry_rva'], 0x800, imports)
    
    dispatch_va = None
    driver_obj_reg = None
    
    for i, insn in enumerate(entry):
        line = f"  {insn['addr']:#018x}: {insn['mnemonic']:8s} {insn['op_str']}"
        if insn['import']:
            line += f"  ; {insn['import']}"
        
        # Look for stores to +0xE0 (IRP_MJ_DEVICE_CONTROL)
        if insn['mnemonic'] == 'mov' and '+ 0xe0]' in insn['op_str']:
            line += "  ; *** IRP_MJ_DEVICE_CONTROL ***"
            # Check preceding lea for the function address
            for j in range(max(0,i-5), i):
                if entry[j]['mnemonic'] == 'lea' and 'rip' in entry[j]['op_str']:
                    disp = struct.unpack_from("<i", entry[j]['bytes'], len(entry[j]['bytes'])-4)[0]
                    dispatch_va = entry[j]['addr'] + entry[j]['size'] + disp
                    line += f"  ; dispatch at {dispatch_va:#x}"
        
        # Also look for +0x70 (IRP_MJ_CREATE)
        if insn['mnemonic'] == 'mov' and '+ 0x70]' in insn['op_str']:
            line += "  ; *** IRP_MJ_CREATE ***"
        
        # Look for IoCreateDevice, IoCreateSymbolicLink
        if insn['import'] and 'IoCreate' in insn['import']:
            line += "  ; *** DEVICE CREATION ***"
        
        print(line)
    
    # === Analyze the main IOCTL dispatch ===
    if dispatch_va:
        dispatch_rva = dispatch_va - pe['image_base']
        print(f"\n=== IRP_MJ_DEVICE_CONTROL HANDLER at RVA {dispatch_rva:#x} ===")
        dispatch = disasm_at(data, pe, dispatch_rva, 0x2000, imports)
        
        for i, insn in enumerate(dispatch):
            line = f"  {insn['addr']:#018x}: {insn['mnemonic']:8s} {insn['op_str']}"
            if insn['import']:
                line += f"  ; {insn['import']}"
            
            # Highlight comparisons with IOCTL codes
            if insn['mnemonic'] == 'cmp':
                for code in [4, 8, 0xc, 0x10, 0x13, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x30, 0x34, 0x40]:
                    if f", {code:#x}" in insn['op_str'] or f", {code}" in insn['op_str']:
                        line += f"  ; *** IOCTL CODE {code:#x} ***"
            
            # Highlight size checks
            if insn['mnemonic'] in ('cmp', 'test') and any(f", {s:#x}" in insn['op_str'] for s in [0x10, 0x18, 0x20, 0x28, 0x30]):
                line += "  ; *** SIZE CHECK? ***"
            
            print(line)
            
            # Stop at ret
            if insn['mnemonic'] == 'ret' and i > 100:
                # Check if next instruction looks like a new function
                if i + 1 < len(dispatch):
                    next_insn = dispatch[i+1]
                    if next_insn['mnemonic'] in ('int3', 'nop') or (next_insn['mnemonic'] == 'push' and 'rbp' in next_insn['op_str']):
                        break
    else:
        print("\n*** Could not find IRP_MJ_DEVICE_CONTROL handler automatically ***")
        print("Searching for IOCTL code switch tables in all code sections...")
        
        # Search for patterns like: cmp reg, 0x14; je target; cmp reg, 0x10; je other
        for sec in pe['sections']:
            if sec['name'] not in ('.text', 'PAGE', 'INIT'):
                continue
            instrs = disasm_at(data, pe, sec['vrva'], sec['raw_size'], imports)
            
            for i, insn in enumerate(instrs):
                if insn['mnemonic'] != 'cmp':
                    continue
                # Look for comparison with known IOCTL codes
                has_14 = ', 0x14' in insn['op_str']
                has_10 = ', 0x10' in insn['op_str']
                
                if has_14 or has_10:
                    # Check window for other IOCTL codes
                    window = instrs[max(0,i-20):min(len(instrs),i+20)]
                    codes_found = set()
                    for w in window:
                        if w['mnemonic'] == 'cmp':
                            for c in [0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x34, 0x40]:
                                if f", {c:#x}" in w['op_str'] or (c < 10 and f", {c}" in w['op_str']):
                                    codes_found.add(c)
                    
                    if len(codes_found) >= 3:
                        print(f"\n  Found IOCTL switch at {insn['addr']:#x}, codes: {sorted(codes_found)}")
                        ctx_start = max(0, i - 10)
                        ctx_end = min(len(instrs), i + 40)
                        for ci in range(ctx_start, ctx_end):
                            ins = instrs[ci]
                            line = f"    {ins['addr']:#018x}: {ins['mnemonic']:8s} {ins['op_str']}"
                            if ins['import']:
                                line += f"  ; {ins['import']}"
                            print(line)

if __name__ == "__main__":
    main()
