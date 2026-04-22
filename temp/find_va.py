"""Find dwViewAngles RVA via GetViewAngles signature.

Pattern: 4C 8B C1 85 D2 74 08 48 8D 05 ?? ?? ?? ?? C3
   mov rax, rcx
   test edx, edx
   jz +8
   lea rax, [rip+disp32]   ; <-- displacement points to dwViewAngles
   ret

The LEA at sig+9 has its 4-byte displacement at sig+10.
RIP-relative target = (sig + 9 + 7) + disp32 = sig + 14 + disp32
That gives the file offset; convert to RVA via PE section table.
"""
import struct, pefile, sys

dll = r"C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\game\csgo\bin\win64\client.dll"

with open(dll, "rb") as f:
    data = f.read()

# Pattern bytes; '?' positions = 10..13 (4 bytes of disp), and last byte 14 = C3.
# We'll search using regex for speed.
import re
pat = re.compile(b"\x4C\x8B\xC1\x85\xD2\x74\x08\x48\x8D\x05....\xC3", re.DOTALL)
matches = list(pat.finditer(data))
print(f"Found {len(matches)} matches in file:")
pe = pefile.PE(dll, fast_load=True)

def file_off_to_rva(fo):
    for s in pe.sections:
        if s.PointerToRawData <= fo < s.PointerToRawData + s.SizeOfRawData:
            return s.VirtualAddress + (fo - s.PointerToRawData)
    return None

for m in matches[:10]:
    fo = m.start()
    rva = file_off_to_rva(fo)
    # LEA disp at fo+10..fo+14, next instr at fo+14
    disp = struct.unpack_from("<i", data, fo + 10)[0]
    next_rva = file_off_to_rva(fo + 14)
    target_rva = next_rva + disp if next_rva is not None else None
    print(f"  file=0x{fo:X} func_rva=0x{rva:X} disp={disp:#x} dwViewAngles_rva=0x{target_rva:X}")

# Also check the GetViewAngles+SetViewAngle pair area to find dwCSGOInput equivalent
# CCSGOInput::CreateMove pattern (current sig)
print("\nCreateMove '48 8B C4 4C 89 40 18 48 89 48 08 55 53 41 54 41 55':")
cm = re.compile(b"\x48\x8B\xC4\x4C\x89\x40\x18\x48\x89\x48\x08\x55\x53\x41\x54\x41\x55", re.DOTALL)
for m in list(cm.finditer(data))[:5]:
    fo = m.start()
    rva = file_off_to_rva(fo)
    print(f"  file=0x{fo:X} rva=0x{rva:X} (image va=0x180000000+rva = 0x{0x180000000+rva:X})")
