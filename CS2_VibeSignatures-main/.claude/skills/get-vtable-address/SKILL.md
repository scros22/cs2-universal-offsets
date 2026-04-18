---
name: get-vtable-address
description: |
  Find a function's vtable address using IDA Pro MCP. Use this skill when want to get exact virtual address of a class.
  Triggers: get vftable address, get virtual function table, find vtable, get vtable
---

# Get VTable address and size

Find a class's virtual function table by class name. Get its address and size in a single step.

## Prerequisites

- ClassName

## Method

### 1. Get vtable address and size

Run this single Python script using `mcp__ida-pro-mcp__py_eval`, replacing `<CLASS_NAME>` with the actual class name (e.g., `CGameRules`, `CCSPlayer_ItemServices`):

```python
mcp__ida-pro-mcp__py_eval code="""
import ida_bytes, ida_name, idaapi, idautils, ida_segment

class_name = "<CLASS_NAME>"
ptr_size = 8 if idaapi.inf_is_64bit() else 4

vtable_start = None  # address of first vfunc pointer
vtable_symbol = ""
is_linux = False
method = ""

# ── Direct symbol lookup ──────────────────────────────────────────────
# Windows: ??_7ClassName@@6B@
win_name = f"??_7{class_name}@@6B@"
addr = ida_name.get_name_ea(idaapi.BADADDR, win_name)
if addr != idaapi.BADADDR:
    vtable_start = addr
    vtable_symbol = win_name
    is_linux = False
    method = "direct"

# Linux: _ZTV<len>ClassName  (e.g. _ZTV10CGameRules)
if vtable_start is None:
    linux_name = f"_ZTV{len(class_name)}{class_name}"
    addr = ida_name.get_name_ea(idaapi.BADADDR, linux_name)
    if addr != idaapi.BADADDR:
        vtable_start = addr + 0x10  # skip offset-to-top + typeinfo ptr
        vtable_symbol = f"{linux_name} + 0x10"
        is_linux = True
        method = "direct"

# ── RTTI / TypeInfo fallback ──────────────────────────────────────────
# Windows: ??_R4ClassName@@6B@ (Complete Object Locator)
if vtable_start is None:
    col_name = f"??_R4{class_name}@@6B@"
    col_addr = ida_name.get_name_ea(idaapi.BADADDR, col_name)
    if col_addr != idaapi.BADADDR:
        is_linux = False
        rdata_seg = ida_segment.get_segm_by_name(".rdata")
        for ref in idautils.DataRefsTo(col_addr):
            if rdata_seg and not (rdata_seg.start_ea <= ref < rdata_seg.end_ea):
                continue
            vtable_start = ref + ptr_size
            sym = ida_name.get_name(vtable_start) or f"??_7{class_name}@@6B@"
            vtable_symbol = sym
            method = "rtti_fallback"
            break

# Linux: _ZTI<len>ClassName (typeinfo)
if vtable_start is None:
    ti_name = f"_ZTI{len(class_name)}{class_name}"
    ti_addr = ida_name.get_name_ea(idaapi.BADADDR, ti_name)
    if ti_addr != idaapi.BADADDR:
        is_linux = True
        for ref in idautils.DataRefsTo(ti_addr):
            ott = ida_bytes.get_qword(ref - ptr_size) if ptr_size == 8 else ida_bytes.get_dword(ref - ptr_size)
            if ott == 0:
                vtable_start = ref + ptr_size
                ztv_addr = ref - ptr_size
                ztv_name = ida_name.get_name(ztv_addr) or f"_ZTV{len(class_name)}{class_name}"
                vtable_symbol = f"{ztv_name} + 0x10"
                method = "rtti_fallback"
                break

assert vtable_start is not None, f"Cannot find vtable for {class_name}"

# ── Count virtual functions ───────────────────────────────────────────
count = 0
for i in range(1000):
    ea = vtable_start + i * ptr_size

    # Linux: stop at next vtable / typeinfo symbol boundary
    if is_linux and i > 0:
        name = ida_name.get_name(ea)
        if name and (name.startswith("_ZTV") or name.startswith("_ZTI")):
            break

    ptr_value = ida_bytes.get_qword(ea) if ptr_size == 8 else ida_bytes.get_dword(ea)

    if ptr_value == 0:
        if is_linux:
            count += 1        # NULL = pure virtual placeholder
            continue
        else:
            break              # Windows: NULL = vtable end

    if ptr_value == 0xFFFFFFFFFFFFFFFF:
        break

    func = idaapi.get_func(ptr_value)
    if func is not None:
        count += 1
        continue

    flags = ida_bytes.get_full_flags(ptr_value)
    if ida_bytes.is_code(flags):
        count += 1
        continue

    break  # not a valid function pointer

size_in_bytes = count * ptr_size

print(f"vtable_class: {class_name}")
print(f"vtable_symbol: {vtable_symbol}")
print(f"vtable_va: {hex(vtable_start)}")
print(f"vtable_size: {hex(size_in_bytes)}")
print(f"vtable_numvfuncs: {count}")
print(f"method: {method}")
"""
```

**Lookup order**:
1. Direct symbol: `??_7ClassName@@6B@` (Win) / `_ZTV<len>ClassName` (Linux)
2. RTTI fallback: `??_R4ClassName@@6B@` → DataRefsTo → vtable (Win) / `_ZTI<len>ClassName` → DataRefsTo with offset-to-top==0 → vtable (Linux)

**Platform-specific vfunc counting**:
- **Linux** (`_ZTV` prefix): NULL entries = pure virtual placeholders (keep counting); stops at next `_ZTV`/`_ZTI` symbol
- **Windows** (`??_7` prefix): NULL = vtable end; stops immediately

### 2. Continue with Unfinished Tasks

If we are called by a task from a task list / parent SKILL, restore and continue with the unfinished tasks.

## Output

The skill returns:
- **vtable_class**: The class name
- **vtable_symbol**: The IDA symbol. e.g. `??_7CGameRules@@6B@` or `_ZTV10CGameRules + 0x10`
- **vtable_va**: The start address of the vtable (first vfunc pointer)
- **vtable_size**: Total size of the vtable in bytes
- **vtable_numvfuncs**: Count of virtual function entries in the vtable
- **method**: How the vtable was found (`direct` or `rtti_fallback`)