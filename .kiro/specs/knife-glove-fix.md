# Spec: Fix CS2 Knife/Glove Changer

**Status**: Draft  
**Priority**: Critical  
**Type**: Bugfix  
**Created**: 2026-04-14

---

## Problem Statement

The CS2 knife/glove changer is completely non-functional. Multiple implementation attempts have failed with different symptoms:

**Current Symptoms**:
- ✅ Karambit model visible when dropping gun and picking up knife
- ✅ "★ Karambit | Fade" displays in bottom right
- ✅ Fade skin (golden color) is visible
- ❌ **CANNOT switch to knife normally (press 3)** - must drop gun first
- ❌ When hooks are active: instant lag in-game
- ❌ When switching to knife: game crashes

**Root Cause** (confirmed via IDA Pro analysis):
CS2 uses an inventory slot system that maps weapon defIndex to slots. When we change defIndex from 42/59 → 507 (Karambit), the game can't find the knife in slot 3 anymore, breaking weapon switching.

---

## Requirements

### Functional Requirements

#### FR1: Knife Model Changing
- **Description**: User can select any knife model from the menu
- **Acceptance Criteria**:
  - [ ] User selects knife model from dropdown (Karambit, Butterfly, etc.)
  - [ ] Knife model changes in-game
  - [ ] Model is visible in first-person view
  - [ ] Model is visible in third-person view
  - [ ] Model is visible when dropped on ground

#### FR2: Knife Skin Application
- **Description**: User can apply any skin to the selected knife
- **Acceptance Criteria**:
  - [ ] User selects paint kit from menu
  - [ ] Skin applies correctly to knife model
  - [ ] Wear value affects skin appearance
  - [ ] Seed value affects pattern placement
  - [ ] StatTrak counter displays if enabled

#### FR3: Weapon Switching Works
- **Description**: User can switch to knife normally using keybind
- **Acceptance Criteria**:
  - [ ] Pressing "3" switches to knife immediately
  - [ ] No need to drop gun first
  - [ ] Switching is instant (no lag)
  - [ ] Can switch back to other weapons normally

#### FR4: Glove Model Changing
- **Description**: User can select any glove model from the menu
- **Acceptance Criteria**:
  - [ ] User selects glove model from dropdown
  - [ ] Glove model changes in-game
  - [ ] Gloves visible in first-person view
  - [ ] Gloves visible when holding any weapon

#### FR5: Glove Skin Application
- **Description**: User can apply any skin to the selected gloves
- **Acceptance Criteria**:
  - [ ] User selects paint kit from menu
  - [ ] Skin applies correctly to glove model
  - [ ] Wear value affects skin appearance

### Non-Functional Requirements

#### NFR1: Performance
- **Description**: No lag or performance degradation
- **Acceptance Criteria**:
  - [ ] No FPS drops when hooks are active
  - [ ] No stuttering when switching weapons
  - [ ] Hooks only run during initialization, not every frame

#### NFR2: Stability
- **Description**: No crashes or game instability
- **Acceptance Criteria**:
  - [ ] No crashes when switching to knife
  - [ ] No crashes when changing config mid-game
  - [ ] No crashes when dying/respawning
  - [ ] Proper exception handling in all hooks

#### NFR3: Compatibility
- **Description**: Works with existing features
- **Acceptance Criteria**:
  - [ ] Weapon skins still work correctly
  - [ ] ESP still shows correct weapon icons
  - [ ] Aimbot still functions normally
  - [ ] No conflicts with other hooks

---

## Design

### Architecture Overview

The solution uses a **three-hook coordinated approach**:

1. **UpdateSubclass Hook**: Applies skin via fallback system (NO defIndex modification)
2. **SetModel Hook**: Substitutes model path during loading
3. **SetMeshGroupMask Hook**: Ensures correct mesh visibility

**Key Insight**: We DON'T modify `m_iItemDefinitionIndex` at all. The defIndex stays 42/59 so weapon switching works. We only substitute the model path and apply skins via the fallback system.

### Component Design

#### Component 1: SetMeshGroupMask Hook
**File**: `features/setmeshgroupmask_hook.h` (NEW FILE)

**Purpose**: Control which parts of the knife/glove model are visible

**Function Signature** (from IDA Pro):
```cpp
void __fastcall SetMeshGroupMask(__int64 entity, __int64 mask)
```

**Hook Implementation**:
```cpp
void Hook_SetMeshGroupMask(uintptr_t entity, uint64_t mask)
{
    __try {
        // Validate entity pointer
        if (!entity || entity < 0x10000) goto original;
        
        // Check if this is a knife entity
        if (IsKnifeEntity(entity) && SkinChanger::cfg.knifeEnabled)
        {
            // Use full mask for custom knives (show all mesh groups)
            Original_SetMeshGroupMask(entity, 0xFFFFFFFFFFFFFFFF);
            return;
        }
        
        // Check if this is a glove entity
        if (IsGloveEntity(entity) && SkinChanger::cfg.gloveEnabled)
        {
            // Use full mask for custom gloves
            Original_SetMeshGroupMask(entity, 0xFFFFFFFFFFFFFFFF);
            return;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
    
original:
    Original_SetMeshGroupMask(entity, mask);
}
```

**Signature**: `48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 48 8D 99`  
**Address**: 0x180A329C0 (from IDA Pro analysis)

---

#### Component 2: SetModel Hook (FIXED)
**File**: `features/setmodel_hook_v2.h` (MODIFY EXISTING)

**Changes Required**:
1. Add comprehensive NULL pointer validation
2. Add model manager initialization check
3. Add proper exception handling
4. Add logging for debugging

**Fixed Implementation**:
```cpp
void Hook_SetModel(uintptr_t entity, const char* modelPath)
{
    __try {
        // CRITICAL: Validate ALL pointers before dereferencing
        
        // 1. Validate entity pointer
        if (!entity || entity < 0x10000 || entity > 0x7FFFFFFFFFFF)
        {
            Log("[SetModel] Invalid entity pointer: 0x%llX", entity);
            Original_SetModel(entity, modelPath);
            return;
        }
        
        // 2. Validate modelPath pointer
        if (!modelPath)
        {
            Log("[SetModel] NULL modelPath for entity 0x%llX", entity);
            Original_SetModel(entity, modelPath);
            return;
        }
        
        // 3. Validate modelPath is readable
        __try {
            volatile char test = modelPath[0];  // Test read
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[SetModel] Unreadable modelPath for entity 0x%llX", entity);
            Original_SetModel(entity, modelPath);
            return;
        }
        
        // 4. Only process if knife/glove changer is enabled
        if (!SkinChanger::cfg.enabled) goto original;
        
        // 5. Check if this is a knife model
        if (SkinChanger::cfg.knifeEnabled && IsKnifeModel(modelPath))
        {
            const char* customModel = GetCustomKnifeModel();
            if (customModel)
            {
                Log("[SetModel] Substituting: %s -> %s", modelPath, customModel);
                Original_SetModel(entity, customModel);
                return;
            }
        }
        
        // 6. Check if this is a glove model
        if (SkinChanger::cfg.gloveEnabled && IsGloveModel(modelPath))
        {
            const char* customModel = GetCustomGloveModel();
            if (customModel)
            {
                Log("[SetModel] Substituting: %s -> %s", modelPath, customModel);
                Original_SetModel(entity, customModel);
                return;
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Log("[SetModel] Exception caught - calling original");
    }
    
original:
    Original_SetModel(entity, modelPath);
}
```

---

#### Component 3: UpdateSubclass Hook (FIXED)
**File**: `features/update_subclass_hook.h` (MODIFY EXISTING)

**Changes Required**:
1. **REMOVE** all defIndex modification code
2. Add caching to prevent spam
3. Only process active weapon
4. Only apply skin (no model changes)

**Fixed Implementation**:
```cpp
void Hook_UpdateSubclass(uintptr_t entity)
{
    __try {
        // CRITICAL: Only process LOCAL PLAYER's ACTIVE weapon
        // This prevents spam and only affects the weapon we're holding
        
        uintptr_t localPawn = GameState::GetLocalPawn();
        if (!localPawn || localPawn < 0x10000) goto original;
        
        uintptr_t activeWeapon = Mem::Read<uintptr_t>(localPawn + Offsets::m_pClippingWeapon);
        if (!activeWeapon || activeWeapon != entity) goto original;
        
        // Cache to avoid reprocessing same entity
        static uintptr_t lastEntity = 0;
        static uint16_t lastDefIndex = 0;
        static int lastPaintKit = 0;
        
        uintptr_t item = entity + Offsets::m_AttributeManager + Offsets::m_Item;
        uint16_t currentDefIndex = Mem::Read<uint16_t>(item + Offsets::m_iItemDefinitionIndex);
        
        // Check if already processed
        if (entity == lastEntity && 
            currentDefIndex == lastDefIndex && 
            lastPaintKit == SkinChanger::cfg.knifePaintKit)
        {
            goto original;
        }
        
        // KNIFE: Apply skin ONLY (DON'T modify defIndex!)
        if (SkinChanger::cfg.knifeEnabled && SkinChanger::IsKnife(currentDefIndex))
        {
            // Apply skin via fallback system
            Mem::Write<int32_t>(entity + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.knifePaintKit);
            Mem::Write<int32_t>(entity + Offsets::m_nFallbackSeed, SkinChanger::cfg.knifeSeed);
            Mem::Write<float>(entity + Offsets::m_flFallbackWear, SkinChanger::cfg.knifeWear);
            
            if (SkinChanger::cfg.knifeStatTrak >= 0)
            {
                Mem::Write<int32_t>(entity + Offsets::m_nFallbackStatTrak, SkinChanger::cfg.knifeStatTrak);
            }
            
            Log("[UpdateSubclass] Applied knife skin (defIndex=%d, paintKit=%d)", 
                currentDefIndex, SkinChanger::cfg.knifePaintKit);
            
            lastEntity = entity;
            lastDefIndex = currentDefIndex;
            lastPaintKit = SkinChanger::cfg.knifePaintKit;
        }
        
        // GLOVE: Apply skin ONLY
        if (SkinChanger::cfg.gloveEnabled && IsGloveEntity(entity))
        {
            Mem::Write<int32_t>(entity + Offsets::m_nFallbackPaintKit, SkinChanger::cfg.glovePaintKit);
            Mem::Write<float>(entity + Offsets::m_flFallbackWear, SkinChanger::cfg.gloveWear);
            
            Log("[UpdateSubclass] Applied glove skin (paintKit=%d)", 
                SkinChanger::cfg.glovePaintKit);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Log("[UpdateSubclass] Exception caught");
    }
    
original:
    Original_UpdateSubclass(entity);
}
```

---

### Hook Installation Order

**CRITICAL**: Hooks must be installed in this exact order:

```cpp
// In dllmain.cpp EntryThread():

// 1. UpdateSubclass hook FIRST (skin application)
Log("[EntryThread] Installing UpdateSubclass hook...");
if (!UpdateSubclassHook::Init())
{
    Log("[EntryThread] UpdateSubclass hook FAILED - aborting");
    return;
}
Log("[EntryThread] UpdateSubclass hook OK");

// 2. SetMeshGroupMask hook SECOND (mesh visibility)
Log("[EntryThread] Installing SetMeshGroupMask hook...");
if (!SetMeshGroupMaskHook::Init())
{
    Log("[EntryThread] SetMeshGroupMask hook FAILED - aborting");
    return;
}
Log("[EntryThread] SetMeshGroupMask hook OK");

// 3. SetModel hook LAST (model substitution)
Log("[EntryThread] Installing SetModel hook...");
if (!SetModelHook::Init())
{
    Log("[EntryThread] SetModel hook FAILED - aborting");
    return;
}
Log("[EntryThread] SetModel hook OK");

Log("[EntryThread] All knife/glove hooks installed successfully!");
```

---

## Implementation Tasks

### Task 1: Create SetMeshGroupMask Hook
**File**: `features/setmeshgroupmask_hook.h` (NEW)  
**Estimated Time**: 2 hours  
**Dependencies**: None

**Subtasks**:
- [ ] Create new file with proper header guards
- [ ] Implement hook function with validation
- [ ] Add signature scanning
- [ ] Add MinHook integration
- [ ] Add logging for debugging
- [ ] Add Init() and Shutdown() functions

**Acceptance Criteria**:
- [ ] Hook installs without errors
- [ ] Signature found in client.dll
- [ ] Hook function is called during knife initialization
- [ ] Mesh mask is applied correctly
- [ ] No crashes or exceptions

---

### Task 2: Fix SetModel Hook
**File**: `features/setmodel_hook_v2.h` (MODIFY)  
**Estimated Time**: 3 hours  
**Dependencies**: None

**Subtasks**:
- [ ] Add comprehensive NULL pointer validation
- [ ] Add model manager initialization check
- [ ] Add proper exception handling
- [ ] Add detailed logging
- [ ] Test with various knife models
- [ ] Verify no crashes

**Acceptance Criteria**:
- [ ] No NULL pointer crashes
- [ ] Model path substitution works
- [ ] Logs show correct model paths
- [ ] All knife models load correctly
- [ ] No exceptions or errors

---

### Task 3: Fix UpdateSubclass Hook
**File**: `features/update_subclass_hook.h` (MODIFY)  
**Estimated Time**: 2 hours  
**Dependencies**: None

**Subtasks**:
- [ ] Remove ALL defIndex modification code
- [ ] Add active weapon check
- [ ] Add caching to prevent spam
- [ ] Keep ONLY skin application code
- [ ] Add proper logging
- [ ] Test for spam in logs

**Acceptance Criteria**:
- [ ] No defIndex modification
- [ ] No spam in logs (max 1 log per weapon switch)
- [ ] Skin applies correctly
- [ ] No lag or performance issues
- [ ] Weapon switching works normally

---

### Task 4: Update dllmain.cpp
**File**: `dllmain.cpp` (MODIFY)  
**Estimated Time**: 1 hour  
**Dependencies**: Tasks 1, 2, 3

**Subtasks**:
- [ ] Add SetMeshGroupMaskHook include
- [ ] Install hooks in correct order
- [ ] Add proper error handling
- [ ] Add logging for each hook
- [ ] Update shutdown sequence

**Acceptance Criteria**:
- [ ] All three hooks install successfully
- [ ] Hooks install in correct order
- [ ] Proper error messages if any hook fails
- [ ] Clean shutdown when unloading

---

### Task 5: Testing and Validation
**Estimated Time**: 4 hours  
**Dependencies**: Tasks 1, 2, 3, 4

**Test Cases**:

#### TC1: Individual Hook Testing
- [ ] Test UpdateSubclass hook alone
  - Verify: Skin applies
  - Verify: No lag
  - Verify: No spam in logs
  
- [ ] Test SetModel hook alone
  - Verify: No crashes
  - Verify: Model substitution works
  - Verify: Logs show correct paths
  
- [ ] Test SetMeshGroupMask hook alone
  - Verify: Mesh visibility correct
  - Verify: No visual glitches

#### TC2: Combined Hook Testing
- [ ] Enable all three hooks
  - Verify: Knife model changes
  - Verify: Knife skin applies
  - Verify: Weapon switching works (press 3)
  - Verify: No lag
  - Verify: No crashes

#### TC3: Edge Cases
- [ ] Rapid weapon switching
- [ ] Dropping knife and picking it up
- [ ] Dying and respawning
- [ ] Changing knife config mid-game
- [ ] Testing all knife models
- [ ] Testing all knife skins

#### TC4: Glove Testing
- [ ] Glove model changes
- [ ] Glove skin applies
- [ ] Gloves visible with all weapons
- [ ] No conflicts with knife changer

---

## Success Criteria

### Must Have (P0)
- [ ] Knife model changes correctly
- [ ] Knife skin applies correctly
- [ ] Weapon switching works (press 3)
- [ ] No lag or performance issues
- [ ] No crashes

### Should Have (P1)
- [ ] Glove model changes correctly
- [ ] Glove skin applies correctly
- [ ] All knife models work
- [ ] All glove models work

### Nice to Have (P2)
- [ ] StatTrak counter works
- [ ] Nametags work
- [ ] Stickers work (if applicable)

---

## Risks and Mitigations

### Risk 1: SetModel Hook Still Crashes
**Probability**: Medium  
**Impact**: High  
**Mitigation**: 
- Add even more validation
- Test with debugger attached
- Add fallback to original function on any exception
- Log all parameters before calling original

### Risk 2: Hooks Cause Lag
**Probability**: Low  
**Impact**: High  
**Mitigation**:
- Only process active weapon
- Add caching to prevent reprocessing
- Profile hook execution time
- Optimize hot paths

### Risk 3: Weapon Switching Still Broken
**Probability**: Low  
**Impact**: Critical  
**Mitigation**:
- Verify defIndex is NEVER modified
- Test weapon switching extensively
- Add logging to verify defIndex stays 42/59
- Test with all weapons

### Risk 4: Conflicts with Other Features
**Probability**: Low  
**Impact**: Medium  
**Mitigation**:
- Test with all features enabled
- Verify weapon skins still work
- Verify ESP still works
- Test aimbot functionality

---

## Rollback Plan

If the implementation fails:

1. **Immediate Rollback**:
   - Disable all three hooks in dllmain.cpp
   - Revert to previous working state
   - Document what failed

2. **Partial Rollback**:
   - Keep working hooks enabled
   - Disable only the problematic hook
   - Continue using weapon skins (which work)

3. **Alternative Approach**:
   - Research EquipItemInLoadout hook
   - Investigate inventory system modification
   - Consider client-side only solution

---

## Timeline

**Total Estimated Time**: 12 hours

- **Day 1** (4 hours):
  - Task 1: Create SetMeshGroupMask Hook (2h)
  - Task 2: Fix SetModel Hook (2h)

- **Day 2** (4 hours):
  - Task 2: Fix SetModel Hook (1h remaining)
  - Task 3: Fix UpdateSubclass Hook (2h)
  - Task 4: Update dllmain.cpp (1h)

- **Day 3** (4 hours):
  - Task 5: Testing and Validation (4h)
  - Bug fixes and iteration

---

## References

- **IDA Pro Analysis**: KNIFE_GLOVE_RESEARCH.md
- **Friend's Signatures**: br5rhvh.txt
- **Previous Attempts**:
  - features/update_subclass_hook.h
  - features/setmodel_hook_v2.h
  - features/knife_changer_final.h
- **Working Code**: features/skinchanger_test.h (weapon skins work perfectly)

---

## Notes

- User has broken hand - do ALL work for them, including building
- User confirmed Gemini cheat has this working with many features
- Friend's signatures are verified and working
- IDA Pro MCP is available for analysis
- Build with `.\build_all.bat` after changes
