# IDA Pro Search Plan - Finding Frame Callback for Knife/Glove Changer

## OBJECTIVE
Find the function that's called every frame BEFORE rendering where we can modify knife/glove defIndex.

## SEARCH STRATEGY

### Search 1: Look for "PostDataUpdate" String
```python
# In IDA Pro:
# 1. Open Strings window (Shift+F12)
# 2. Search for "PostDataUpdate"
# 3. Find xrefs to that string
# 4. Trace to function that uses it
```

**Expected Result:**
- String reference in event registration code
- Function pointer or vtable entry
- Actual callback function address

### Search 2: Look for "FrameStageNotify" String
```python
# We already found RTTI string at 0x18216b740
# 1. Find xrefs to this string
# 2. Look for vtable construction
# 3. Find function that implements FrameStageNotify
```

**Expected Result:**
- CSource2Client vtable entry
- FrameStageNotify function address
- Signature for pattern scanning

### Search 3: Analyze CSource2Client Vtable
```python
# CSource2Client vtable at 0x181aaf0f8
# Instance at 0x18230CDD0
# 1. Dump all vtable entries
# 2. Look for functions with frame/update/stage in name
# 3. Analyze promising functions
```

**Expected Result:**
- List of all CSource2Client virtual functions
- Frame-related function candidates
- Decompiled code to verify behavior

### Search 4: Find Event Registration
```python
# Look for event system registration
# 1. Search for "RegisterEventListener" or similar
# 2. Find where frame events are registered
# 3. Trace to callback functions
```

**Expected Result:**
- Event registration code
- List of frame event callbacks
- Function addresses for each event

### Search 5: Analyze UpdateSubclass Callers
```python
# UpdateSubclass at 0x1801e9a70
# 1. Find all xrefs to UpdateSubclass
# 2. Analyze calling functions
# 3. Look for frame-based callers
```

**Expected Result:**
- Functions that call UpdateSubclass
- Context where it's called
- Potential hook points

## DETAILED SEARCH PROCEDURES

### Procedure A: String Search with IDA MCP
```python
# Use find_regex tool to search for frame-related strings
patterns = [
    "PostDataUpdate",
    "PreDataUpdate", 
    "FrameStageNotify",
    "FrameStage",
    "OnFrame",
    "UpdateFrame",
    "ClientFrame"
]

for pattern in patterns:
    results = find_regex(pattern)
    # Analyze xrefs to each result
```

### Procedure B: Vtable Analysis
```python
# Dump CSource2Client vtable
vtable_addr = 0x181aaf0f8
num_functions = 50  # Estimate

for i in range(num_functions):
    func_ptr = read_qword(vtable_addr + i * 8)
    func_name = get_func_name(func_ptr)
    print(f"[{i}] {hex(func_ptr)}: {func_name}")
    
    # Decompile promising functions
    if "frame" in func_name.lower() or "update" in func_name.lower():
        decompile(func_ptr)
```

### Procedure C: Cross-Reference Analysis
```python
# Find what calls our known functions
known_funcs = {
    "UpdateSubclass": 0x1801e9a70,
    "SetModel": 0x1808e19a0,
}

for name, addr in known_funcs.items():
    xrefs = get_xrefs_to(addr)
    print(f"\n{name} called by:")
    for xref in xrefs:
        caller = get_func_name(xref)
        print(f"  {hex(xref)}: {caller}")
```

### Procedure D: Pattern Recognition
```python
# Look for common frame callback patterns
patterns = [
    # Function called with frame count parameter
    "mov edx, [frameCount]",
    "call [callback]",
    
    # Loop over game systems
    "mov rcx, [pGameSystem]",
    "call [vtable+offset]",
    
    # Event dispatch
    "mov ecx, [eventType]",
    "call [DispatchEvent]"
]
```

## VERIFICATION CRITERIA

For each candidate function, verify:

### 1. Call Frequency
```python
# Set breakpoint in IDA debugger
# Check if called every frame (60+ times per second)
```

### 2. Call Timing
```python
# Verify it's called BEFORE rendering
# Check call stack to confirm position in frame
```

### 3. Access to Entities
```python
# Verify we can access local player
# Verify we can access weapon entities
# Verify we can modify defIndex
```

### 4. Stability
```python
# Verify function is stable across game updates
# Check if it's a core engine function
# Verify signature is unique
```

## EXPECTED FINDINGS

### Best Case: OnPostDataUpdate
```cpp
// Signature: TBD (need to find)
// Address: TBD
// Called: Every frame after network update
// Perfect for modifying defIndex before render
```

### Alternative: CreateMove
```cpp
// Signature: TBD
// Address: TBD  
// Called: Every input frame
// Good for modifying defIndex
```

### Fallback: Present Hook
```cpp
// Hook D3D11 Present
// Modify defIndex before each frame render
// Less elegant but guaranteed to work
```

## IMPLEMENTATION AFTER FINDING

Once we find the function:

```cpp
// 1. Generate signature
const char* sig = "48 89 5C 24 ? 48 89 6C 24 ? ...";

// 2. Find address
uintptr_t addr = FindPattern(clientBase, sig);

// 3. Create hook
MH_CreateHook(addr, Hook_Function, &Original_Function);

// 4. In hook, modify defIndex
void Hook_Function(...) {
    ModifyKnifeGlove();
    Original_Function(...);
}
```

## SUCCESS CRITERIA

We've succeeded when:
1. ✅ Found function address in IDA
2. ✅ Verified it's called every frame
3. ✅ Verified timing is correct (before render)
4. ✅ Generated working signature
5. ✅ Implemented hook
6. ✅ Knife model changes in-game
7. ✅ Glove model changes in-game

## TOOLS NEEDED

1. **IDA Pro** - Open client.dll
2. **IDA MCP Server** - Remote analysis via Kiro
3. **DebugView++** - Monitor debug output
4. **CS2** - Test changes in-game

## TIME ESTIMATE

- String searches: 30 minutes
- Vtable analysis: 1 hour
- Function verification: 1 hour
- Implementation: 30 minutes
- Testing: 30 minutes

**Total: ~3.5 hours of focused IDA work**

## NOTES

- Don't implement until we VERIFY the function works
- Use IDA debugger to confirm call frequency
- Test with weapon skins first (known working)
- Document everything for future updates

---

This is a SYSTEMATIC approach - no guessing, only verification.
