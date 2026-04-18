"""
SIVX64.sys RE v5 — Full dump of IRP_MJ_DEVICE_CONTROL handler at VA 0x21984
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
        chars = struct.unpack_from("<I", data, off + 36)[0]
        sections.append({'name': name, 'vrva': vrva, 'vsize': vsize,
                        'raw_ptr': raw_ptr, 'raw_size': raw_size, 'chars': chars})
    import_rva = struct.unpack_from("<I", data, opt + 120)[0]
    return {'image_base': image_base, 'entry_rva': entry_rva,
            'sections': sections, 'import_rva': import_rva}

def rva_to_offset(sections, rva):
    for s in sections:
        if s['vrva'] <= rva < s['vrva'] + s['raw_size']:
            return rva - s['vrva'] + s['raw_ptr']
    return None

def parse_imports(data, pe):
    imports = {}
    off = rva_to_offset(pe['sections'], pe['import_rva'])
    if off is None: return imports
    while True:
        ilt_rva = struct.unpack_from("<I", data, off)[0]
        name_rva = struct.unpack_from("<I", data, off + 12)[0]
        iat_rva = struct.unpack_from("<I", data, off + 16)[0]
        if ilt_rva == 0 and name_rva == 0: break
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

def resolve_call(insn_bytes, insn_addr, insn_size, pe, imports):
    if len(insn_bytes) >= 6:
        disp = struct.unpack_from("<i", bytes(insn_bytes), len(insn_bytes) - 4)[0]
        target_va = insn_addr + insn_size + disp
        target_rva = target_va - pe['image_base']
        if target_rva in imports:
            return imports[target_rva]
    return None

def main():
    data = read_driver()
    pe = parse_pe(data)
    imports = parse_imports(data, pe)
    ib = pe['image_base']
    
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True
    
    # IRP_MJ_DEVICE_CONTROL at VA 0x21984, RVA 0x11984
    dispatch_rva = 0x11984
    # Dump 0x8000 bytes (~32KB) to capture the full function
    foff = rva_to_offset(pe['sections'], dispatch_rva)
    code = data[foff:foff+0x8000]
    base = ib + dispatch_rva
    
    print(f"=== IRP_MJ_DEVICE_CONTROL handler at VA {base:#x} ===")
    print(f"RVA: {dispatch_rva:#x}, File offset: {foff:#x}")
    print()
    
    func_depth = 0  # Track push/pop balance
    ret_count = 0
    
    for insn in md.disasm(code, base):
        imp = None
        if insn.mnemonic in ('call', 'jmp') and 'qword ptr [rip' in insn.op_str:
            imp = resolve_call(insn.bytes, insn.address, insn.size, pe, imports)
        
        mk = ""
        if imp:
            mk = f"  ; {imp}"
        
        # Resolve LEA targets
        if insn.mnemonic == 'lea' and 'rip' in insn.op_str:
            d = struct.unpack_from("<i", bytes(insn.bytes), len(insn.bytes)-4)[0]
            tgt = insn.address + insn.size + d
            mk += f"  ; VA {tgt:#x}"
        
        # Resolve direct CALL targets
        if insn.mnemonic == 'call' and insn.op_str.startswith('0x'):
            mk += f"  ; CALL sub_{int(insn.op_str, 16):#x}"
        
        # Highlight IoControlCode access patterns
        if insn.mnemonic in ('cmp', 'sub') and 'dword' in insn.op_str:
            # Extract immediate values from the instruction for IOCTL detection
            op = insn.op_str
            # Check for hex immediates
            if ', 0x' in op:
                val_str = op.split(', 0x')[-1].rstrip(']')
                try:
                    val = int(val_str, 16)
                    if val in range(0, 0x50):
                        mk += f"  ; *** CMD {val:#x}? ***"
                except: pass
        
        # Highlight specific comparisons
        if insn.mnemonic == 'cmp':
            op = insn.op_str
            # Check for known IOCTL sub-commands
            for cmd in [0x04, 0x08, 0x0c, 0x10, 0x13, 0x14, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x30, 0x34, 0x38, 0x3c, 0x40]:
                if f", {cmd:#x}" in op or f", {cmd}" in op:
                    mk += f"  ; === CMD CHECK {cmd:#x} ==="
        
        # Highlight size checks
        if insn.mnemonic == 'cmp' and 'ebx' in insn.op_str:
            mk += " ; SIZE CHECK?"
        
        # STATUS codes
        if insn.mnemonic == 'mov' and '0xc0000' in insn.op_str:
            val_str = insn.op_str.split('0xc0000')[-1]
            try:
                val = int('c0000' + val_str, 16)
                status_names = {
                    0xc0000004: 'STATUS_INFO_LENGTH_MISMATCH',
                    0xc000000d: 'STATUS_INVALID_PARAMETER',
                    0xc0000010: 'STATUS_INFO_LENGTH_MISMATCH',
                    0xc0000022: 'STATUS_ACCESS_DENIED',
                    0xc0000023: 'STATUS_BUFFER_TOO_SMALL',
                    0xc00000bb: 'STATUS_NOT_SUPPORTED',
                    0xc00000c0: 'STATUS_DEVICE_DOES_NOT_EXIST',
                }
                name = status_names.get(val, f'NTSTATUS')
                mk += f"  ; {name}"
            except: pass
        
        # Highlight buffer operations
        if insn.mnemonic == 'mov' and '+ 0x38]' in insn.op_str:
            if 'r15' in insn.op_str or 'rsi' in insn.op_str:
                mk += "  ; IoStatus.Information?"
        if insn.mnemonic == 'mov' and '+ 0x30]' in insn.op_str:
            if 'rsi' in insn.op_str or 'r15' in insn.op_str:
                mk += "  ; IoStatus.Status?"
        
        print(f"  {insn.address:#018x}: {insn.mnemonic:8s} {insn.op_str}{mk}")
        
        # Function end detection
        if insn.mnemonic == 'ret':
            ret_count += 1
            # Stop when we hit return followed by int3 padding
            next_off = insn.address + insn.size - base + foff
            if next_off < len(data) and data[rva_to_offset(pe['sections'], insn.address + insn.size - ib)] == 0xCC:
                # Check if this is aligned like a function boundary
                if ret_count >= 1 and insn.address > base + 0x1000:
                    print("  --- potential function end ---")
                    # Keep going for a bit to verify
                    next_count = 0
                    for j in range(10):
                        off = rva_to_offset(pe['sections'], insn.address + insn.size + j - ib)
                        if off and data[off] == 0xCC:
                            next_count += 1
                    if next_count >= 3:
                        print("  === END OF DEVICE_CONTROL HANDLER ===")
                        break

if __name__ == "__main__":
    main()
