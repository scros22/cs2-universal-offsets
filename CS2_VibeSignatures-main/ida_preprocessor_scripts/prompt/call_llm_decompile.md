You are a reverse engineering expert. I have disassembly outputs and procedure code of the same function.

This is the function for reference:

**Disassembly for Reference**

```c
{disasm_for_reference}
```

**Procedure code for Reference**

```c
{procedure_for_reference}
```

This is the function you need to reverse-engineering:

**Disassembly to reverse-engineering**

```c
{disasm_code}
```

**Procedure code to reverse-engineering**

```c
{procedure}
```

Please collect all references to "{symbol_name_list}" in the function you need to reverse-engineering and output those references as YAML.

Example:

```yaml
found_vcall: # This is for indirect call to virtual function or virtual function pointer fetching.
  - insn_va: '0x180777700'               # Always be the instruction with displacement offset
    insn_disasm: call    [rax+68h]       # Always be the instruction with displacement offset
    vfunc_offset: '0x68'
    func_name: ILoopMode_OnLoopActivate
  - insn_va: '0x180777778'               # Always be the instruction with displacement offset
    insn_disasm: mov     rax, [rax+80h]  # Always be the instruction with displacement offset
    vfunc_offset: '0x80'
    func_name: INetworkMessages_GetNetworkGroupCount
found_call: # This is for direct call to non-virtual regular function.
  - insn_va: '0x180888800'
    insn_disasm: call    sub_180999900
    func_name: CLoopModeGame_RegisterEventMapInternal
  - insn_va: '0x180888880'
    insn_disasm: call    sub_180555500
    func_name: CLoopModeGame_SetGameSystemState
found_funcptr: # This is for non-virtual regular function pointer.
  - insn_va: '0x180666600'                # Must load/reference the function pointer target address
    insn_disasm: lea     rdx, sub_15BC910 # Must load/reference the function pointer target address
    funcptr_name: CLoopModeGame_OnClientPollNetworking
found_gv: # This is for reference to global variable.
  - insn_va: '0x180444400'
    insn_disasm: mov     rcx, cs:qword_180666600
    gv_name: s_GameEventManager
found_struct_offset: # This is for reference to struct offset.
  - insn_va: '0x1801BA12A'                # Always be the instruction with displacement offset
    insn_disasm: mov     rcx, [r14+58h]   # Always be the instruction with displacement offset
    offset: '0x58'
    size: 8
    struct_name: CGameResourceService
    member_name: m_pEntitySystem
```

If nothing found, output an empty YAML. DO NOT output anything other than the desired YAML. DO NOT collect unrelated symbols.
