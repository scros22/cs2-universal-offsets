---
name: find-CServerSideClient_IsHearingClient
description: Find and identify the CServerSideClient_IsHearingClient function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 engine2.dll or libengine2.so to locate the IsHearingClient function by cross-referencing CSVCMsg_PeerList_t vtable and analyzing the bidirectional hearing check pattern.
disable-model-invocation: true
---

# Find CServerSideClient_IsHearingClient

Locate `CServerSideClient_IsHearingClient` in CS2 engine2.dll or libengine2.so using IDA Pro MCP tools.

## Method

### 1. Get CSVCMsg_PeerList_t VTable Address

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CSVCMsg_PeerList_t`.

If the skill returns an error, **STOP** and report to user.

Otherwise, extract `vtable_va` for subsequent steps.

### 2. Cross-Reference CSVCMsg_PeerList_t VTable

Find functions that reference the `CSVCMsg_PeerList_t` vtable:

```
mcp__ida-pro-mcp__xrefs_to addrs="<vtable_va>"
```

This should return references from a single function (the PeerList broadcast function). Note the function address.

### 3. Decompile the Referencing Function

```
mcp__ida-pro-mcp__decompile addr="<referencing_function_addr>"
```

### 4. Identify IsHearingClient VFunc Offset from Code Pattern

In the decompiled output, look for the **bidirectional hearing check pattern** — two consecutive virtual function calls using the same vtable offset on two different `CServerSideClient` pointers:

```c
v30 = (*(__int64 (__fastcall **)(...)(*(_QWORD *)v18 + <VFUNC_OFFSET>))(v18, v24[18], ...);
*(_DWORD *)(v25 + 16) |= 4u;
*(_BYTE *)(v25 + 40) = v30;
v31 = (*(__int64 (__fastcall **)(...)(*(_QWORD *)v24 + <VFUNC_OFFSET>))(v24, v18[18]);
*(_DWORD *)(v25 + 16) |= 8u;
*(_BYTE *)(v25 + 41) = v31;
```

Extract `<VFUNC_OFFSET>` (e.g. `152LL` = `0x98`). Calculate the vtable index: `index = <VFUNC_OFFSET> / 8` (e.g. `152 / 8 = 19`).

### 5. Get CServerSideClient VTable and Resolve Function Address

**ALWAYS** Use SKILL `/get-vtable-from-yaml` with `class_name=CServerSideClient`.

If the skill returns an error, **STOP** and report to user.

Otherwise, use `vtable_entries[<index>]` to get the `IsHearingClient` function address.

### 6. Decompile and Verify

```
mcp__ida-pro-mcp__decompile addr="<IsHearingClient_addr>"
```

Confirm the function matches this characteristic pattern:
```c
if ( a2 == *(_DWORD *)(a1 + 72) )
  return *(_BYTE *)(a1 + 3824);
if ( a2 < 0 || (v4 = *(_QWORD *)(a1 + 80), a2 >= *(_DWORD *)(v4 + 592)) )
  v5 = 0LL;
else
  v5 = *(_QWORD **)(*(_QWORD *)(v4 + 600) + 8LL * a2);
```

With the bitmask lookup at the end:
```c
v7 = *((_DWORD *)v5 + ((__int64)*(int *)(a1 + 72) >> 5) + 756);
return _bittest(&v7, *(_DWORD *)(a1 + 72) & 0x1F);
```

### 7. Rename the Function

```
mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<IsHearingClient_addr>", "name": "CServerSideClient_IsHearingClient"}]}
```

### 8. Generate and Validate Unique Signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

### 9. Write IDA Analysis Output as YAML

**ALWAYS** Use SKILL `/write-vfunc-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `CServerSideClient_IsHearingClient`
- `func_addr`: The function address from step 5
- `func_sig`: The validated signature from step 8

VTable parameters:
- `vtable_name`: `CServerSideClient`
- `vfunc_offset`: `<VFUNC_OFFSET>` in hex (e.g. `0x98`)
- `vfunc_index`: The calculated index (e.g. `19`)

## Function Characteristics

- **Parameters**: `(this, client_index)` where `this` is CServerSideClient pointer, `client_index` is the target client slot
- **Return**: `unsigned __int8` (bool) - whether this client can hear the specified client
- **Key offsets**:
  - `+0x48`: Client slot index
  - `+0x50`: Server pointer
  - `+0xEF0`: Self hearing state (Windows)
  - `+0x250`: Max clients count (in server struct)
  - `+0x258`: Client array pointer (in server struct)

## Identification Pattern

The function checks:
1. If target client is self, return self hearing state
2. Get target client from server's client array
3. Check HLTV replay conditions
4. Return bit from hearing bitmask

## Output YAML Format

The output YAML filename depends on the platform:
- `engine2.dll` → `CServerSideClient_IsHearingClient.windows.yaml`
- `libengine2.so` → `CServerSideClient_IsHearingClient.linux.yaml`
