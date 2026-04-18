---
name: find-CNetworkGameServer_GetFreeClient
description: Find and identify the CNetworkGameServer_GetFreeClient function in CS2 binary using IDA Pro MCP. Use this skill when reverse engineering CS2 engine2.dll or libengine2.so to locate the GetFreeClient function by searching for the "NETWORK_DISCONNECT_REJECT_SERVERFULL to %s: Cannot get free client" debug string reference and analyzing cross-references.
disable-model-invocation: true
---

# Find CNetworkGameServer_GetFreeClient

Locate `CNetworkGameServer_GetFreeClient` in CS2 engine2.dll or libengine2.so using IDA Pro MCP tools.

## Method

1. Search for the debug string:
   ```
   mcp__ida-pro-mcp__find_regex pattern="NETWORK_DISCONNECT_REJECT_SERVERFULL.*Cannot get free client"
   ```

2. Get cross-references to the string:
   ```
   mcp__ida-pro-mcp__xrefs_to addrs="<string_addr>"
   ```

3. Decompile the referencing function:
   ```
   mcp__ida-pro-mcp__decompile addr="<function_addr>"
   ```

4. Identify the CNetworkGameServer_GetFreeClient function:
   - In the decompiled code, look for the call that appears just before the "Cannot get free client" error path:
     ```c
     v88 = sub_XXXXXX(a1, (_OWORD *)a3, 0LL, v111, 0, v108);
     if ( !v88 )
     {
       // ... disconnect with NETWORK_DISCONNECT_REJECT_SERVERFULL
       // ... "NETWORK_DISCONNECT_REJECT_SERVERFULL to %s: Cannot get free client\n"
     }
     ```
   - The `sub_XXXXXX` called right before the null check and "Cannot get free client" log is `CNetworkGameServer_GetFreeClient`

5. Rename the function:
   ```
   mcp__ida-pro-mcp__rename batch={"func": [{"addr": "<function_addr>", "name": "CNetworkGameServer_GetFreeClient"}]}
   ```

6. Generate and validate unique signature:

   **ALWAYS** Use SKILL `/generate-signature-for-function` to generate a robust and unique signature for the function.

7. Write IDA analysis output as YAML beside the binary:

   **ALWAYS** Use SKILL `/write-func-as-yaml` to write the analysis results.

   Required parameters:
   - `func_name`: `CNetworkGameServer_GetFreeClient`
   - `func_addr`: The function address from step 4
   - `func_sig`: The validated signature from step 6

## Signature Pattern

The parent function (CNetworkGameServerBase::ConnectClient) references these debug strings:
```
"CNetworkGameServerBase::ConnectClient( name='%s', remote='%s' )\n"
"NETWORK_DISCONNECT_REJECT_SERVERFULL to %s: Cannot get free client\n"
```

The target function is called within ConnectClient, just before the "Cannot get free client" error handling block.

## Function Characteristics

- **Parameters**: `(this, addr, unk, steamid, unk2, out_buf)` where `this` is CNetworkGameServer pointer
- **Return**: Pointer to a free client slot, or NULL if server is full
- **Purpose**: Allocates and returns a free client slot from the server's client list
- **Module**: engine2.dll / libengine2.so

## Output YAML Format

The output YAML filename depends on the platform:
- `engine2.dll` → `CNetworkGameServer_GetFreeClient.windows.yaml`
- `libengine2.so` → `CNetworkGameServer_GetFreeClient.linux.yaml`
