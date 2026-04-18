"""
SIVX64.sys RE v3 — Find the real IRP_MJ_DEVICE_CONTROL handler
Approach: find MajorFunction[14] assignment in DriverEntry chain,
search for IofCompleteRequest call sites, and find IOCTL code extraction.
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

def main():
    data = read_driver()
    pe = parse_pe(data)
    imports = parse_imports(data, pe)
    ib = pe['image_base']
    
    print("=== PE SECTIONS ===")
    for s in pe['sections']:
        print(f"  {s['name']:10s} RVA={s['vrva']:#08x} VSize={s['vsize']:#08x} RawPtr={s['raw_ptr']:#08x} RawSize={s['raw_size']:#08x} Chars={s['chars']:#010x}")
    
    # Build IAT reverse lookup
    iat_by_name = {}
    for rva, name in imports.items():
        iat_by_name[name] = ib + rva
    
    print(f"\nIofCompleteRequest IAT VA: {iat_by_name.get('ntoskrnl.exe!IofCompleteRequest', 0):#x}")
    print(f"IoIs32bitProcess IAT VA: {iat_by_name.get('ntoskrnl.exe!IoIs32bitProcess', 0):#x}")
    
    # === STEP 1: Find all MajorFunction assignments ===
    # Binary search for byte pattern: mov [reg + 0xE0], reg
    # Encoding: 48 89 XX E0 00 00 00 where XX encodes src/dst registers
    # Also: REX.W variants with 4C prefix
    print("\n=== BINARY SEARCH: MajorFunction[14] (offset 0xE0) assignments ===")
    
    # Look for all patterns: mov qword ptr [reg + disp32], reg where disp32 = 0xE0
    # 48 89 8X E0 00 00 00  (mov [reg + 0xE0], rcx)  for various X
    # 48 89 XX E0 00 00 00  general pattern
    # 4C 89 XX E0 00 00 00  for r8-r15 source
    targets = []
    for prefix in [0x48, 0x4C]:
        for modrm in range(0x80, 0xC0):  # ModRM with 32-bit displacement
            pattern = bytes([prefix, 0x89, modrm, 0xE0, 0x00, 0x00, 0x00])
            idx = 0
            while True:
                idx = data.find(pattern, idx)
                if idx == -1: break
                rva = offset_to_rva(pe['sections'], idx)
                if rva:
                    reg_dst = modrm & 7
                    reg_src = (modrm >> 3) & 7
                    reg_names = ['rax','rcx','rdx','rbx','rsp','rbp','rsi','rdi']
                    if prefix == 0x4C:
                        src_name = f"r{8+reg_src}"
                    else:
                        src_name = reg_names[reg_src]
                    dst_name = reg_names[reg_dst]
                    print(f"  File offset {idx:#x}, RVA {rva:#x}, VA {ib+rva:#x}: mov [{dst_name}+0xE0], {src_name}")
                    targets.append((rva, ib + rva))
                idx += 1
    
    # Also search +0x70 (IRP_MJ_CREATE = 0)
    print("\n=== MajorFunction[0] (IRP_MJ_CREATE, offset 0x70) assignments ===")
    for prefix in [0x48, 0x4C]:
        for modrm in range(0x40, 0x80):  # ModRM with 8-bit displacement (0x70 fits in 1 byte)
            if (modrm & 7) == 4: continue  # SIB byte
            pattern = bytes([prefix, 0x89, modrm, 0x70])
            idx = 0
            while True:
                idx = data.find(pattern, idx)
                if idx == -1: break
                rva = offset_to_rva(pe['sections'], idx)
                if rva:
                    reg_dst = modrm & 7
                    reg_src = (modrm >> 3) & 7
                    reg_names = ['rax','rcx','rdx','rbx','rsp','rbp','rsi','rdi']
                    if prefix == 0x4C:
                        src_name = f"r{8+reg_src}"
                    else:
                        src_name = reg_names[reg_src]
                    dst_name = reg_names[reg_dst]
                    print(f"  File offset {idx:#x}, RVA {rva:#x}, VA {ib+rva:#x}: mov [{dst_name}+0x70], {src_name}")
                idx += 1
    
    # === STEP 2: For each MajorFunction target, disassemble context ===
    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True
    
    for rva, va in targets:
        print(f"\n=== CONTEXT around MajorFunction[14] assignment at VA {va:#x} ===")
        # Disassemble 200 bytes before and 100 bytes after
        start_rva = max(rva - 200, 0)
        foff = rva_to_offset(pe['sections'], start_rva)
        if foff is None: continue
        code = data[foff:foff+400]
        base = ib + start_rva
        
        for insn in md.disasm(code, base):
            imp = None
            if insn.mnemonic in ('call', 'jmp') and 'qword ptr [rip' in insn.op_str:
                imp = resolve_call(insn.bytes, insn.address, insn.size, pe, imports)
            
            marker = ""
            if insn.address == va:
                marker = " <<< IRP_MJ_DEVICE_CONTROL"
            if imp: marker += f"  ; {imp}"
            
            # Check if this is a LEA loading a function address
            if insn.mnemonic == 'lea' and 'rip' in insn.op_str:
                disp = struct.unpack_from("<i", bytes(insn.bytes), len(insn.bytes)-4)[0]
                target = insn.address + insn.size + disp
                marker += f"  ; loads VA {target:#x}"
            
            print(f"  {insn.address:#018x}: {insn.mnemonic:8s} {insn.op_str}{marker}")
            
            if insn.address > va + 100:
                break

    # === STEP 3: Find all IofCompleteRequest call sites ===
    print("\n=== IofCompleteRequest call sites (first 20) ===")
    iocomplete_iat_rva = None
    for rva_k, name in imports.items():
        if 'IofCompleteRequest' in name:
            iocomplete_iat_rva = rva_k
            break
    
    if iocomplete_iat_rva:
        # RIP-relative call: FF 15 xx xx xx xx  (call qword ptr [rip + disp])
        # The displacement = iat_va - (insn_va + 6)
        count = 0
        for sec in pe['sections']:
            if not (sec['chars'] & 0x20000000): continue  # CODE section
            foff = sec['raw_ptr']
            code = data[foff:foff+sec['raw_size']]
            base = ib + sec['vrva']
            
            i = 0
            while i < len(code) - 6:
                if code[i] == 0xFF and code[i+1] == 0x15:
                    disp = struct.unpack_from("<i", code, i+2)[0]
                    target_rva = (sec['vrva'] + i + 6 + disp)
                    if target_rva == iocomplete_iat_rva:
                        call_va = base + i
                        call_rva = sec['vrva'] + i
                        if count < 20:
                            print(f"  Call at VA {call_va:#x} (RVA {call_rva:#x})")
                        count += 1
                    i += 6
                else:
                    i += 1
            
        print(f"  Total IofCompleteRequest calls: {count}")
    
    # === STEP 4: Follow DriverEntry chain ===
    # Entry at RVA 0x32cd4, jumps to VA 0x42008 (RVA 0x32008)
    print("\n=== REAL DriverEntry at RVA 0x32008 (first 200 instructions) ===")
    foff = rva_to_offset(pe['sections'], 0x32008)
    if foff:
        code = data[foff:foff+0x1000]
        base = ib + 0x32008
        icount = 0
        for insn in md.disasm(code, base):
            imp = None
            if insn.mnemonic in ('call', 'jmp') and 'qword ptr [rip' in insn.op_str:
                imp = resolve_call(insn.bytes, insn.address, insn.size, pe, imports)
            
            marker = ""
            if imp: marker += f"  ; {imp}"
            if insn.mnemonic == 'lea' and 'rip' in insn.op_str:
                disp = struct.unpack_from("<i", bytes(insn.bytes), len(insn.bytes)-4)[0]
                target = insn.address + insn.size + disp
                marker += f"  ; VA {target:#x}"
            if '+ 0xe0]' in insn.op_str:
                marker += "  ; *** MajorFunction[14] ***"
            if '+ 0x70]' in insn.op_str and insn.mnemonic == 'mov':
                marker += "  ; *** MajorFunction[0]? ***"
            
            print(f"  {insn.address:#018x}: {insn.mnemonic:8s} {insn.op_str}{marker}")
            icount += 1
            if icount >= 200: break
    
    # === STEP 5: What's at 0x128d8? The earlier "dispatch" function ===
    # Check what IOCTL code this function handles by looking at who calls it
    print("\n=== XREF search: who calls 0x128d8? ===")
    target_rva = 0x128d8 - ib  # Hmm, 0x128d8 is the VA
    # Actually the function VA is 0x128d8 which = RVA 0x028d8 if ib=0x10000
    # Wait, the VA would be ib + 0x28d8 = 0x138d8? No...
    # From previous output: "IOCTL Dispatch function (starts at 0x128d8)"
    # That was using VA = ib + rva = 0x10000 + rva
    # So if listed as 0x128d8, then rva = 0x128d8 - 0x10000 = 0x28d8
    
    func_rva = 0x128d8 - ib  
    
    # Search for direct relative call: E8 xx xx xx xx
    count = 0
    for sec in pe['sections']:
        if not (sec['chars'] & 0x20000000): continue
        foff = sec['raw_ptr']
        code = data[foff:foff+sec['raw_size']]
        
        for i in range(len(code) - 5):
            if code[i] == 0xE8:
                disp = struct.unpack_from("<i", code, i+1)[0]
                call_target_rva = sec['vrva'] + i + 5 + disp
                if call_target_rva == func_rva:
                    caller_va = ib + sec['vrva'] + i
                    print(f"  Call from VA {caller_va:#x} (RVA {sec['vrva'] + i:#x}) → 0x{ib + func_rva:#x}")
                    count += 1
        
        # Also search for LEA to get address
        for i in range(len(code) - 7):
            if code[i] in (0x48, 0x4C) and code[i+1] == 0x8D:
                modrm = code[i+2]
                if (modrm & 0xC7) == 0x05:  # [rip + disp32]
                    disp = struct.unpack_from("<i", code, i+3)[0]
                    lea_target_rva = sec['vrva'] + i + 7 + disp
                    if lea_target_rva == func_rva:
                        lea_va = ib + sec['vrva'] + i
                        print(f"  LEA from VA {lea_va:#x} (RVA {sec['vrva'] + i:#x}) loads addr of 0x{ib + func_rva:#x}")
                        count += 1
    
    if count == 0:
        print("  No direct calls/LEAs found — may be called via function pointer or jump table")
    
    # === STEP 6: Search for the actual IOCTL code extraction pattern ===
    # In IRP_MJ_DEVICE_CONTROL, the driver gets IoStackLocation->Parameters.DeviceIoControl.IoControlCode
    # This is at offset 0x18 in the Parameters union of the IO_STACK_LOCATION
    # After IoGetCurrentIrpStackLocation (IRP->CurrentStackLocation at +0xB8):
    # IoControlCode = StackLocation->Parameters.DeviceIoControl.IoControlCode
    # The IoControlCode is at StackLocation + 0x18
    # Input buffer length at +0x10, Output buffer length at +0x08
    # Then the driver typically does: and IOCTL, mask; shr IOCTL, 2 to get function number
    # Or: sub IOCTL, base_code; cmp IOCTL, max
    
    print("\n=== Search for IOCTL code extraction (IoControlCode at +0x18 from stack location) ===")
    # Look for: mov reg, [reg + 0x18] followed by comparison/switch
    # Combined with: mov reg, [irp + 0xB8] (CurrentStackLocation)
    
    # Actually, we should look for the standard pattern:
    # mov rax, [rdx + 0xB8]  (get current stack location from IRP)
    # where rdx is the IRP parameter of IRP_MJ_DEVICE_CONTROL(DeviceObject, IRP)
    
    # Let's search for 0xB8 displacement loads which could be CurrentStackLocation
    for sec in pe['sections']:
        if not (sec['chars'] & 0x20000000): continue
        foff = sec['raw_ptr']
        code = data[foff:foff+sec['raw_size']]
        base = ib + sec['vrva']
        
        for i in range(len(code) - 7):
            # REX.W mov reg, [reg + 0xB8]  = 48 8B XX B8 00 00 00
            if code[i] in (0x48, 0x4C) and code[i+1] == 0x8B:
                modrm = code[i+2]
                if (modrm & 0xC0) == 0x80:  # 32-bit displacement
                    reg_rm = modrm & 7
                    if reg_rm == 4: continue  # SIB
                    disp = struct.unpack_from("<i", code, i+3)[0]
                    if disp == 0xB8:
                        va = base + i
                        reg_names = ['rax','rcx','rdx','rbx','rsp','rbp','rsi','rdi']
                        reg_src = reg_names[reg_rm]
                        reg_dst = (modrm >> 3) & 7
                        if code[i] == 0x4C:
                            dst_name = f"r{8+reg_dst}"
                        else:
                            dst_name = reg_names[reg_dst]
                        print(f"  VA {va:#x}: mov {dst_name}, [{reg_src} + 0xB8]  (possible CurrentStackLocation)")
                        
                        # Disassemble context 
                        ctx_start = max(0, i - 100)
                        ctx_code = code[ctx_start:i+200]
                        ctx_base = base + ctx_start
                        print(f"  Context:")
                        hit_shown = False
                        for insn in md.disasm(ctx_code, ctx_base):
                            imp = None
                            if insn.mnemonic in ('call', 'jmp') and 'qword ptr [rip' in insn.op_str:
                                imp = resolve_call(insn.bytes, insn.address, insn.size, pe, imports)
                            mk = ""
                            if imp: mk = f"  ; {imp}"
                            if insn.address == va: mk += " <<< HERE"
                            if insn.mnemonic == 'lea' and 'rip' in insn.op_str:
                                d = struct.unpack_from("<i", bytes(insn.bytes), len(insn.bytes)-4)[0]
                                mk += f"  ; VA {insn.address + insn.size + d:#x}"
                            if '+ 0x18]' in insn.op_str: mk += " ; IoControlCode?"
                            if '+ 0x10]' in insn.op_str: mk += " ; InputBufferLength?"
                            if '+ 8]' in insn.op_str and insn.mnemonic == 'mov': mk += " ; OutputBufferLength?"
                            
                            if insn.address >= va - 50:
                                print(f"    {insn.address:#018x}: {insn.mnemonic:8s} {insn.op_str}{mk}")
                                hit_shown = True
                            if hit_shown and insn.address > va + 150:
                                break
                        print()

if __name__ == "__main__":
    main()
