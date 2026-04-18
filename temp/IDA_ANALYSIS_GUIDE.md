# IDA Pro Analysis Guide for CS2 Knife/Glove Changing

## What We Need to Find

We have these signatures that supposedly work:

1. **SetModel**: `40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24 40`
2. **SetMeshGroupMask**: `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99`
3. **UpdateSubclass**: `4C 8B DC 53 48 81 EC ? ? ? ? 48 8B 41`

## Steps in IDA Pro 9.3

### 1. Load client.dll
- File → Open → Navigate to CS2 game folder
- Path: `C:\Program Files (x86)\Steam\steamapps\common\Counter-Strike Global Offensive\game\csgo\bin\win64\client.dll`
- Let IDA analyze it (this will take a few minutes)

### 2. Search for SetModel Function

**Method 1: Binary Search**
- Alt+B (Binary Search)
- Paste: `40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24 40`
- Replace `?` with `..` (two dots for wildcard)
- Click "Find"

**Method 2: Text Search**
- Alt+T (Text Search)
- Search for: "SetModel"
- Look for function that matches the signature

### 3. Analyze SetModel Function

Once found, check:
- **Parameters**: What does it take? (entity pointer, model path string?)
- **Calling convention**: __fastcall? (RCX = first param, RDX = second param)
- **Return type**: void? pointer?
- **Cross-references**: Where is it called from? How do other functions use it?

### 4. Find UpdateSubclass Function

Same process:
- Binary search: `4C 8B DC 53 48 81 EC ? ? ? ? 48 8B 41`
- Check parameters and calling convention
- Look at cross-references to see how it's used

### 5. Find SetMeshGroupMask Function

- Binary search: `48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D 99`
- Analyze parameters and usage

## What to Look For

### Critical Questions:

1. **SetModel Parameters**:
   - Is first param (RCX) the weapon entity or something else?
   - Is second param (RDX) a string pointer to model path?
   - Are there any other hidden parameters?

2. **UpdateSubclass Parameters**:
   - Does it take the item pointer or weapon pointer?
   - Does it need to be called before or after SetModel?

3. **Calling Sequence**:
   - Find a place in the code where these functions are called
   - What order are they called in?
   - Are there any other functions called between them?

4. **Model Precaching**:
   - Is there a model precache/load function that needs to be called first?
   - Look for strings like "PrecacheModel" or "LoadModel"

## Export Your Findings

Once you find the functions, document:

```cpp
// SetModel function analysis
// Address: 0x????????
// Signature: 40 53 48 83 EC ? 48 8B D9 4C 8B C2 48 8B 0D ? ? ? ? 48 8D 54 24 40
// Prototype: void __fastcall SetModel(uintptr_t entity, const char* modelPath)
// Notes: [any special behavior you notice]

// UpdateSubclass function analysis  
// Address: 0x????????
// Signature: 4C 8B DC 53 48 81 EC ? ? ? ? 48 8B 41
// Prototype: void __fastcall UpdateSubclass(uintptr_t item)
// Notes: [any special behavior you notice]
```

## Quick IDA Tips

- **F5**: Decompile function (Hex-Rays)
- **X**: Cross-references to/from
- **N**: Rename function/variable
- **Y**: Change function prototype
- **G**: Jump to address
- **Space**: Switch between graph/text view

## What We're Trying to Solve

Our code calls these functions but knives don't change. Possible issues:
1. Wrong parameters (entity vs item pointer)
2. Wrong calling order
3. Missing precache step
4. Need to call additional functions
5. Timing issue (need to call at specific moment)

Find out HOW the game itself calls these functions when loading knife models!
