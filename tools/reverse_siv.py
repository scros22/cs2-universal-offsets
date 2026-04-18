"""
SIVX64.sys Reverse Engineering Script
Finds and analyzes the IRP_MJ_DEVICE_CONTROL handler to determine
exact IOCTL buffer structures for all commands.
"""

import struct
import sys
from capstone import Cs, CS_ARCH_X86, CS_MODE_64

DRIVER_PATH = r"c:\Users\Samuel\License-Loader\Loader\Products\CS2\SIVX64.sys"

def read_driver():
    with open(DRIVER_PATH, "rb") as f:
        return f.read()

def parse_pe(data):
    """Parse PE headers to get sections and entry point"""
    e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
    
    # PE signature
    sig = struct.unpack_from("<I", data, e_lfanew)[0]
    assert sig == 0x4550, f"Bad PE sig: {sig:#x}"
    
    coff = e_lfanew + 4
    num_sections = struct.unpack_from("<H", data, coff + 2)[0]
    opt_hdr_size = struct.unpack_from("<H", data, coff + 16)[0]
    
    opt = coff + 20
    # Check PE32+ (optional header magic)
    magic = struct.unpack_from("<H", data, opt)[0]
    assert magic == 0x20B, f"Not PE32+: {magic:#x}"
    
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
        sections.append({
            'name': name, 'vrva': vrva, 'vsize': vsize,
            'raw_ptr': raw_ptr, 'raw_size': raw_size
        })
    
    # Import directory
    import_rva = struct.unpack_from("<I", data, opt + 120)[0]
    import_size = struct.unpack_from("<I", data, opt + 124)[0]
    
    return {
        'image_base': image_base,
        'entry_rva': entry_rva,
        'sections': sections,
        'import_rva': import_rva,
        'import_size': import_size,
        'e_lfanew': e_lfanew,
    }

def rva_to_offset(sections, rva):
    for s in sections:
        if s['vrva'] <= rva < s['vrva'] + s['raw_size']:
            return rva - s['vrva'] + s['raw_ptr']
    return None

def parse_imports(data, pe):
    """Get imported functions"""
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
        
        # DLL name
        name_off = rva_to_offset(pe['sections'], name_rva)
        if name_off:
            dll_name = data[name_off:data.index(b'\x00', name_off)].decode('ascii', errors='replace')
        else:
            dll_name = "???"
        
        # Parse ILT entries
        ilt_off = rva_to_offset(pe['sections'], ilt_rva)
        iat_off_base = iat_rva  # RVA of IAT entries
        idx = 0
        if ilt_off:
            while True:
                entry = struct.unpack_from("<Q", data, ilt_off + idx * 8)[0]
                if entry == 0:
                    break
                
                if entry & (1 << 63):  # Ordinal
                    func_name = f"Ord#{entry & 0xFFFF}"
                else:
                    hint_off = rva_to_offset(pe['sections'], entry & 0x7FFFFFFF)
                    if hint_off:
                        func_name = data[hint_off+2:data.index(b'\x00', hint_off+2)].decode('ascii', errors='replace')
                    else:
                        func_name = f"???"
                
                iat_entry_rva = iat_rva + idx * 8
                imports[iat_entry_rva] = f"{dll_name}!{func_name}"
                idx += 1
        
        off += 20  # Next import descriptor
    
    return imports

def disasm_range(data, pe, rva_start, size, imports=None):
    """Disassemble a range of code"""
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True
    
    file_off = rva_to_offset(pe['sections'], rva_start)
    if file_off is None:
        return []
    
    code = data[file_off:file_off + size]
    base = pe['image_base'] + rva_start
    
    instructions = []
    for insn in md.disasm(code, base):
        ins_str = f"  {insn.address:#018x}: {insn.mnemonic:8s} {insn.op_str}"
        
        # Annotate calls to imports
        if imports and insn.mnemonic in ('call', 'jmp') and insn.op_str.startswith('qword ptr [rip'):
            # Calculate target RIP-relative address
            # The operand is [rip + disp], rip = insn.address + insn.size
            if len(insn.bytes) >= 6:
                # Extract displacement (last 4 bytes of instruction typically)
                disp = struct.unpack_from("<i", bytes(insn.bytes), len(insn.bytes) - 4)[0]
                target_va = insn.address + insn.size + disp
                target_rva = target_va - pe['image_base']
                if target_rva in imports:
                    ins_str += f"  ; {imports[target_rva]}"
        
        instructions.append((insn.address, insn.mnemonic, insn.op_str, ins_str))
    
    return instructions

def find_ioctl_dispatch(data, pe, imports):
    """Find the IRP_MJ_DEVICE_CONTROL handler by analyzing DriverEntry"""
    entry_rva = pe['entry_rva']
    print(f"\n{'='*70}")
    print(f"DriverEntry at RVA {entry_rva:#x} (VA {pe['image_base'] + entry_rva:#x})")
    print(f"{'='*70}")
    
    # Disassemble DriverEntry
    instrs = disasm_range(data, pe, entry_rva, 0x400, imports)
    
    print("\n--- DriverEntry disassembly ---")
    for addr, mnem, ops, s in instrs[:80]:
        print(s)
    
    # Look for MajorFunction table setup
    # Pattern: mov qword ptr [rcx/rdx + offset], rax/r8/etc
    # MajorFunction offsets: IRP_MJ_DEVICE_CONTROL = 14 * 8 + 0x70 = 0xE0
    # DRIVER_OBJECT.MajorFunction starts at offset 0x70
    # IRP_MJ_CREATE = 0, offset = 0x70
    # IRP_MJ_CLOSE = 2, offset = 0x80
    # IRP_MJ_DEVICE_CONTROL = 14, offset = 0xE0
    
    dispatch_handlers = {}
    for addr, mnem, ops, s in instrs:
        if mnem == 'mov' and 'qword ptr [r' in ops:
            # Check for MajorFunction offsets
            for major, name in [(0x70, 'IRP_MJ_CREATE'), (0x80, 'IRP_MJ_CLOSE'), 
                                (0xE0, 'IRP_MJ_DEVICE_CONTROL'),
                                (0x78, 'IRP_MJ_CREATE_NAMED_PIPE'),
                                (0x90, 'IRP_MJ_READ'), (0x98, 'IRP_MJ_WRITE')]:
                hex_off = f"+ {major:#x}]" if major < 0x100 else f"+ {major:#x}]"
                if hex_off in ops or f"+ 0x{major:x}]" in ops:
                    print(f"\n  *** Found {name} setup at {addr:#x}: {ops}")
                    dispatch_handlers[name] = (addr, ops)
    
    return dispatch_handlers

def find_all_lea_targets(data, pe, imports):
    """Scan all code sections for function references and switch tables"""
    print(f"\n{'='*70}")
    print("Scanning all code for IOCTL dispatch patterns...")
    print(f"{'='*70}")
    
    for sec in pe['sections']:
        if not (sec['name'] in ('.text', 'PAGE', 'INIT', '.code')):
            continue
        
        print(f"\n--- Section: {sec['name']} (RVA {sec['vrva']:#x}, size {sec['raw_size']:#x}) ---")
        
        instrs = disasm_range(data, pe, sec['vrva'], sec['raw_size'], imports)
        
        # Find comparison with IOCTL codes (0x04, 0x08, 0x0C, 0x10, 0x14, etc.)
        ioctl_refs = []
        for i, (addr, mnem, ops, s) in enumerate(instrs):
            # Look for cmp with small values (IOCTL codes)
            if mnem == 'cmp':
                for code in [0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24, 0x28, 0x40]:
                    if f", {code:#x}" in ops or f", {code}" in ops:
                        # Print surrounding context
                        start = max(0, i - 3)
                        end = min(len(instrs), i + 8)
                        ioctl_refs.append((addr, code, instrs[start:end]))
            
            # Also look for sub/add with these values (switch table optimization)
            if mnem in ('sub', 'add') and any(f", {c:#x}" in ops or f", {c}" in ops 
                                               for c in [0x04, 0x10, 0x14]):
                start = max(0, i - 2)
                end = min(len(instrs), i + 10)
                ioctl_refs.append((addr, 0, instrs[start:end]))
        
        if ioctl_refs:
            print(f"\n  Found {len(ioctl_refs)} IOCTL code references:")
            for addr, code, context in ioctl_refs:
                print(f"\n  --- IOCTL {code:#x} reference at {addr:#x} ---")
                for _, _, _, s in context:
                    print(f"  {s}")

def find_size_checks(data, pe, imports):
    """Find buffer size validation patterns near IOCTL code comparisons"""
    print(f"\n{'='*70}")
    print("Searching for buffer size checks (InputBufferLength/OutputBufferLength)...")
    print(f"{'='*70}")
    
    for sec in pe['sections']:
        if sec['name'] not in ('.text', 'PAGE', 'INIT', '.code'):
            continue
        
        instrs = disasm_range(data, pe, sec['vrva'], sec['raw_size'], imports)
        
        # Look for cmp with typical buffer sizes
        for i, (addr, mnem, ops, s) in enumerate(instrs):
            if mnem == 'cmp':
                # Check for specific size values commonly used for struct validation
                for sz in [0x10, 0x14, 0x18, 0x1C, 0x20, 0x24, 0x28, 0x30, 0x38, 0x40]:
                    if f", {sz:#x}" in ops:
                        # Check if there's an IOCTL-related comparison nearby
                        start = max(0, i - 15)
                        end = min(len(instrs), i + 5)
                        nearby = [ops2 for _, _, ops2, _ in instrs[start:end]]
                        is_ioctl_context = any(
                            any(f", {c:#x}" in o for c in [0x04, 0x08, 0x10, 0x14, 0x20])
                            for o in nearby
                        )
                        if is_ioctl_context:
                            print(f"\n  Buffer size check: cmp ..., {sz:#x} at {addr:#x}")
                            ctx_start = max(0, i - 5)
                            ctx_end = min(len(instrs), i + 5)
                            for _, _, _, s2 in instrs[ctx_start:ctx_end]:
                                print(f"    {s2}")

def find_mmmap_calls(data, pe, imports):
    """Find calls to MmMapIoSpace, MmCopyMemory, etc."""
    print(f"\n{'='*70}")
    print("Searching for memory mapping API calls...")
    print(f"{'='*70}")
    
    target_funcs = ['MmMapIoSpace', 'MmCopyMemory', 'MmGetPhysicalAddress',
                    'MmUnmapIoSpace', 'MmGetSystemRoutineAddress',
                    'IoAllocateMdl', 'MmBuildMdlForNonPagedPool', 'MmMapLockedPages',
                    'MmProbeAndLockPages', 'MmMapLockedPagesSpecifyCache']
    
    # Check imports
    for rva, name in imports.items():
        for target in target_funcs:
            if target in name:
                print(f"  Import: {name} at IAT RVA {rva:#x}")
    
    # Also search for strings referencing these via MmGetSystemRoutineAddress
    raw = data
    for func in target_funcs:
        # Search for ASCII string
        needle = func.encode('ascii')
        idx = 0
        while True:
            idx = raw.find(needle, idx)
            if idx == -1:
                break
            print(f"  String '{func}' at file offset {idx:#x}")
            idx += 1
        
        # Search for Unicode string
        needle_u = func.encode('utf-16-le')
        idx = 0
        while True:
            idx = raw.find(needle_u, idx)
            if idx == -1:
                break
            print(f"  Unicode string '{func}' at file offset {idx:#x}")
            idx += 1

def dump_dispatch_function(data, pe, imports):
    """Find IRP_MJ_DEVICE_CONTROL handler and dump its full disassembly"""
    print(f"\n{'='*70}")
    print("Full code section disassembly (looking for dispatch pattern)...")
    print(f"{'='*70}")
    
    for sec in pe['sections']:
        if sec['name'] not in ('.text', 'PAGE', '.code'):
            continue
        
        instrs = disasm_range(data, pe, sec['vrva'], sec['raw_size'], imports)
        
        # Find the IOCTL dispatch function — look for the pattern:
        # comparison with 0x10 (Cmd_READ_PHYS) or 0x14 (Cmd_MAP_RW)
        # followed by conditional jumps
        found_dispatch = False
        for i, (addr, mnem, ops, s) in enumerate(instrs):
            if mnem == 'cmp' and ', 0x10' in ops:
                # Check if there are other IOCTL comparisons nearby
                window = instrs[max(0,i-30):min(len(instrs),i+50)]
                has_14 = any('0x14' in o for _, _, o, _ in window)
                has_04 = any('0x4' in o and 'cmp' == m for _, m, o, _ in window)
                
                if has_14 or has_04:
                    found_dispatch = True
                    # This is likely the dispatch function
                    # Find the function start (look backwards for a common prologue)
                    func_start = i
                    for j in range(i, max(0, i-100), -1):
                        if instrs[j][1] in ('push', 'sub') and 'rsp' in instrs[j][2]:
                            func_start = j
                            break
                        # Also check for 'mov [rsp+' which is common in MSVC
                        if instrs[j][1] == 'mov' and '[rsp' in instrs[j][2] and j < func_start:
                            func_start = j
                    
                    # Print from function start to ~200 instructions after
                    end = min(len(instrs), func_start + 300)
                    print(f"\n  === IOCTL Dispatch function (starts at {instrs[func_start][0]:#x}) ===")
                    for _, _, _, s2 in instrs[func_start:end]:
                        print(s2)
                    print(f"\n  === End of dispatch dump ===")
                    break
        
        if found_dispatch:
            break

def main():
    print("SIVX64.sys Reverse Engineering")
    print(f"File: {DRIVER_PATH}")
    
    data = read_driver()
    print(f"Size: {len(data)} bytes ({len(data)/1024:.1f} KB)")
    
    pe = parse_pe(data)
    print(f"Image base: {pe['image_base']:#x}")
    print(f"Entry RVA: {pe['entry_rva']:#x}")
    print(f"Sections:")
    for s in pe['sections']:
        print(f"  {s['name']:8s} RVA={s['vrva']:#08x} VSize={s['vsize']:#08x} "
              f"Raw={s['raw_ptr']:#08x} RawSize={s['raw_size']:#08x}")
    
    imports = parse_imports(data, pe)
    print(f"\nImports ({len(imports)} functions):")
    for rva, name in sorted(imports.items()):
        print(f"  IAT {rva:#06x}: {name}")
    
    # Find memory mapping calls (imports + dynamic resolution strings)
    find_mmmap_calls(data, pe, imports)
    
    # Analyze DriverEntry
    find_ioctl_dispatch(data, pe, imports)
    
    # Find IOCTL code comparisons in all code
    find_all_lea_targets(data, pe, imports)
    
    # Find buffer size validation 
    find_size_checks(data, pe, imports)
    
    # Full dispatch function dump
    dump_dispatch_function(data, pe, imports)

if __name__ == "__main__":
    main()
