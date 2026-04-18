"""
SIVX64.sys RE v4 — Focus on finding MmMapIoSpace usage and the physical read code path.
Also: trace from IofCompleteRequest back to find dispatch handler.
Also dump DriverEntry fully to find MajorFunction assignment.
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

def offset_to_rva(sections, foffset):
    for s in sections:
        if s['raw_ptr'] <= foffset < s['raw_ptr'] + s['raw_size']:
            return foffset - s['raw_ptr'] + s['vrva']
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

def disasm_range(data, pe, rva, size, imports=None):
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True
    foff = rva_to_offset(pe['sections'], rva)
    if foff is None: return []
    code = data[foff:foff+size]
    base = pe['image_base'] + rva
    result = []
    for insn in md.disasm(code, base):
        imp = None
        if imports and insn.mnemonic in ('call', 'jmp') and 'qword ptr [rip' in insn.op_str:
            imp = resolve_call(insn.bytes, insn.address, insn.size, pe, imports)
        result.append({
            'addr': insn.address, 'rva': insn.address - pe['image_base'],
            'mnemonic': insn.mnemonic, 'op_str': insn.op_str,
            'bytes': bytes(insn.bytes), 'size': insn.size, 'import': imp
        })
    return result

def main():
    data = read_driver()
    pe = parse_pe(data)
    imports = parse_imports(data, pe)
    ib = pe['image_base']
    
    # Build import lookup
    iat_by_name = {}
    for rva, name in imports.items():
        iat_by_name[name] = rva  # IAT RVA
    
    # === STEP 1: Find MmGetSystemRoutineAddress calls ===
    print("=== STEP 1: MmGetSystemRoutineAddress call sites ===")
    mmgsra_iat = iat_by_name.get('ntoskrnl.exe!MmGetSystemRoutineAddress')
    if mmgsra_iat:
        for sec in pe['sections']:
            if not (sec['chars'] & 0x20000000): continue
            foff = sec['raw_ptr']
            code = data[foff:foff+sec['raw_size']]
            for i in range(len(code) - 6):
                if code[i] == 0xFF and code[i+1] == 0x15:
                    disp = struct.unpack_from("<i", code, i+2)[0]
                    target_rva = sec['vrva'] + i + 6 + disp
                    if target_rva == mmgsra_iat:
                        call_va = ib + sec['vrva'] + i
                        call_rva = sec['vrva'] + i
                        print(f"\n  Call at VA {call_va:#x} (RVA {call_rva:#x})")
                        # Disassemble context to see what string is being resolved
                        ctx_rva = max(call_rva - 40, sec['vrva'])
                        instrs = disasm_range(data, pe, ctx_rva, 150, imports)
                        for ins in instrs:
                            mk = ""
                            if ins['import']: mk = f"  ; {ins['import']}"
                            if ins['mnemonic'] == 'lea' and 'rip' in ins['op_str']:
                                d = struct.unpack_from("<i", ins['bytes'], len(ins['bytes'])-4)[0]
                                tgt = ins['addr'] + ins['size'] + d
                                tgt_off = rva_to_offset(pe['sections'], tgt - ib)
                                if tgt_off:
                                    # Try to read string (unicode)
                                    s = data[tgt_off:tgt_off+100]
                                    if s[1] == 0 and s[0] != 0:  # unicode
                                        try:
                                            ustr = s.split(b'\x00\x00')[0].decode('utf-16-le', errors='replace')
                                            mk += f"  ; \"{ustr}\""
                                        except: pass
                                    elif s[0] != 0:  # ascii
                                        try:
                                            astr = s.split(b'\x00')[0].decode('ascii', errors='replace')
                                            mk += f"  ; \"{astr}\""
                                        except: pass
                                mk += f"  ; VA {tgt:#x}"
                            if ins['addr'] == call_va:
                                mk += " <<< MmGetSystemRoutineAddress"
                            # After the call, check if result is stored
                            if ins['mnemonic'] == 'mov' and 'rip' in ins['op_str'] and ins['addr'] > call_va and ins['addr'] < call_va + 20:
                                d = struct.unpack_from("<i", ins['bytes'], len(ins['bytes'])-4)[0]
                                store_va = ins['addr'] + ins['size'] + d
                                mk += f"  ; STORES RESULT to VA {store_va:#x} (RVA {store_va - ib:#x})"
                            print(f"    {ins['addr']:#018x}: {ins['mnemonic']:8s} {ins['op_str']}{mk}")
    
    # === STEP 2: Find MmUnmapIoSpace calls (IAT import) ===
    print("\n\n=== STEP 2: MmUnmapIoSpace call sites ===")
    mmunmap_iat = iat_by_name.get('ntoskrnl.exe!MmUnmapIoSpace')
    if mmunmap_iat:
        for sec in pe['sections']:
            if not (sec['chars'] & 0x20000000): continue
            foff = sec['raw_ptr']
            code = data[foff:foff+sec['raw_size']]
            count = 0
            for i in range(len(code) - 6):
                if code[i] == 0xFF and code[i+1] == 0x15:
                    disp = struct.unpack_from("<i", code, i+2)[0]
                    target_rva = sec['vrva'] + i + 6 + disp
                    if target_rva == mmunmap_iat:
                        call_va = ib + sec['vrva'] + i
                        print(f"  MmUnmapIoSpace call at VA {call_va:#x}")
                        count += 1
            if count:
                print(f"  Total in {sec['name']}: {count}")

    # === STEP 3: Search for calls via dynamically resolved function pointer ===
    # MmMapIoSpace resolved at runtime is stored in a global variable
    # Find all indirect calls: call qword ptr [rip + xxx] where the target is .data
    # Actually, let's find stores after MmGetSystemRoutineAddress calls and then find reads from same location
    
    # === STEP 4: Dump the ENTIRE DriverEntry to find MajorFunction ===
    print("\n\n=== STEP 4: Full DriverEntry (everything until return) ===")
    start_rva = 0x32008
    instrs = disasm_range(data, pe, start_rva, 0x1800, imports)
    
    for i, ins in enumerate(instrs):
        mk = ""
        if ins['import']: mk = f"  ; {ins['import']}"
        if ins['mnemonic'] == 'lea' and 'rip' in ins['op_str']:
            d = struct.unpack_from("<i", ins['bytes'], len(ins['bytes'])-4)[0]
            tgt = ins['addr'] + ins['size'] + d
            mk += f"  ; VA {tgt:#x}"
        # Highlight MajorFunction-related
        for mf_off in [0x70, 0x78, 0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0, 0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0]:
            if f"+ {mf_off:#x}]" in ins['op_str'] or f"+ 0x{mf_off:x}]" in ins['op_str']:
                idx = (mf_off - 0x70) // 8
                irp_names = {0:'CREATE',2:'CLOSE',3:'READ',4:'WRITE',12:'CLEANUP',14:'DEVICE_CONTROL',15:'INTERNAL_DC',16:'SHUTDOWN',22:'POWER',27:'PNP'}
                name = irp_names.get(idx, f"MF[{idx}]")
                mk += f"  ; *** MajorFunction: {name} ***"
        # Check DriverUnload
        if '+ 0x68]' in ins['op_str'] and ins['mnemonic'] == 'mov':
            mk += "  ; *** DriverUnload? ***"
            
        print(f"  {ins['addr']:#018x}: {ins['mnemonic']:8s} {ins['op_str']}{mk}")
        
        if ins['mnemonic'] == 'ret' and i > 50:
            next_ok = False
            if i + 1 < len(instrs):
                if instrs[i+1]['mnemonic'] in ('int3', 'nop'):
                    next_ok = True
            if next_ok or i > 300:
                print(f"  --- END OF DriverEntry ---")
                break
    
    # === STEP 5: Analyze ALL functions that reference the IRP ===
    # Look for IoControlCode extraction: [stack_loc + 0x18]
    # The MajorFunction array index doesn't just index Device Control
    # Perhaps the driver uses a single dispatch for all IRP types
    print("\n\n=== STEP 5: Functions reading IoControlCode from stack location ([reg + 0x18]) ===")
    
    # The main dispatch function should:
    # 1. Get stack location from IRP (offset 0xB8 in IRP)
    # 2. Read IoControlCode from [stack_loc + 0x18]
    # 3. Switch on function code
    
    # Let's look around each IofCompleteRequest call for the dispatch logic
    for ioc_rva in [0x1186d, 0x11965, 0x18ea2]:
        print(f"\n  --- Context around IofCompleteRequest at RVA {ioc_rva:#x} (VA {ib+ioc_rva:#x}) ---")
        # Find function start by scanning backwards for int3/nop padding
        func_start = ioc_rva
        foff = rva_to_offset(pe['sections'], ioc_rva)
        if foff is None: continue
        for back in range(1, 0x2000):
            check_off = rva_to_offset(pe['sections'], ioc_rva - back)
            if check_off is None: break
            if data[check_off] == 0xCC:  # int3
                func_start = ioc_rva - back + 1
                break
        
        print(f"  Function likely starts at RVA {func_start:#x} (VA {ib+func_start:#x})")
        instrs = disasm_range(data, pe, func_start, min(0x3000, ioc_rva - func_start + 0x200), imports)
        
        # Only print first 80 instructions to keep output manageable 
        for ins in instrs[:80]:
            mk = ""
            if ins['import']: mk = f"  ; {ins['import']}"
            if ins['mnemonic'] == 'lea' and 'rip' in ins['op_str']:
                d = struct.unpack_from("<i", ins['bytes'], len(ins['bytes'])-4)[0]
                tgt = ins['addr'] + ins['size'] + d
                mk += f"  ; VA {tgt:#x}"
            if '+ 0x18]' in ins['op_str']: mk += " ; ***IoControlCode?***"
            if '+ 0x10]' in ins['op_str'] and ins['mnemonic'] in ('mov', 'movzx'): mk += " ; InputBufLen?"
            if '+ 8]' in ins['op_str'] and ins['mnemonic'] in ('mov', 'movzx'): mk += " ; OutputBufLen?"
            if '+ 0xb8]' in ins['op_str']: mk += " ; CurrentStackLocation?"
            if ins['addr'] == ib + ioc_rva:
                mk += " <<< IofCompleteRequest"
            print(f"    {ins['addr']:#018x}: {ins['mnemonic']:8s} {ins['op_str']}{mk}")

if __name__ == "__main__":
    main()
