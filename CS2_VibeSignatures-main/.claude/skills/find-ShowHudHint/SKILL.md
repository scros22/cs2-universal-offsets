---
name: find-ShowHudHint
description: Find and identify the ShowHudHint function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 server.dll or libserver.so to locate the ShowHudHint function by searching for its string pointer in the data section and resolving the function pointer at string_ptr + 0x10.
disable-model-invocation: true
---

# Find ShowHudHint

Locate `ShowHudHint` in CS2 server.dll or libserver.so using IDA Pro MCP tools.

## Method

### 1. Search for the string

```
mcp__ida-pro-mcp__find_regex pattern="ShowHudHint"
```

### 2. Get cross-references to the string

```
mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
```

### 3. Identify the function pointer from the data section

Look for a cross-reference in the `.data` section (not in code). The data layout is:

**Windows:**
```
.data:XXXXXXXX     dq offset aShowhudhint   ; "ShowHudHint"
.data:XXXXXXXX+8   align 10h
.data:XXXXXXXX+10  dq offset sub_YYYYYYYY   ; <-- ShowHudHint
```

**Linux:**
```
.data:XXXXXXXX     dq offset aShowhudhint   ; "ShowHudHint"
.data:XXXXXXXX+8   align 10h
.data:XXXXXXXX+10  dq offset sub_YYYYYYYY   ; <-- ShowHudHint
```

Read the function pointer at `string_ptr_addr + 0x10`:

```
mcp__ida-pro-mcp__get_int queries={"addr": "<string_ptr_addr + 0x10>", "ty": "u64"}
```

### 4. Decompile and verify the function

```
mcp__ida-pro-mcp__decompile addr="<function_addr>"
```

Verify this is the ShowHudHint function.

### 5. Rename the function

```
mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "ShowHudHint"}]}
```

### 6. Generate and validate unique signature

**ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

### 7. Write IDA analysis output as YAML

**ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

Required parameters:
- `func_name`: `ShowHudHint`
- `func_addr`: The function address from step 3
- `func_sig`: The validated signature from step 6

## String Reference Pattern

The function is located by finding the entity input descriptor in the `.data` section:
- String pointer at offset +0x00
- Padding/alignment at offset +0x08
- Function pointer at offset +0x10

| String | Function |
|--------|----------|
| `"ShowHudHint"` | `ShowHudHint` |

## Function Characteristics

- **Type**: Entity input handler
- **Location method**: String pointer + 0x10 offset in `.data` section

## Output YAML Format

The output YAML filename depends on the platform:
- `server.dll` → `ShowHudHint.windows.yaml`
- `libserver.so` / `libserver.so` → `ShowHudHint.linux.yaml`
