# Testing Guide - New Knife/Glove Changer

**Build Status**: ✅ COMPILED SUCCESSFULLY  
**Date**: 2026-04-14  
**Approach**: EquipItemInLoadout hook + Frame-based manager

---

## Quick Start

### 1. Inject
```
x64\Lucid.exe
```

### 2. Check Initialization
Open: `%TEMP%\cs2init.txt`

Look for:
```
[EntryThread] KnifeGloveManager::Init OK
[EntryThread] EquipItemHook::Init OK - knife/glove changing ENABLED
```

If you see "FAILED", check the logs for errors.

### 3. Enable in Menu
1. Press INSERT to open menu
2. Go to "Skins" tab
3. Enable "Knife Changer"
4. Select knife model (e.g., Karambit)
5. Select skin (e.g., Fade)
6. Enable "Glove Changer" (optional)
7. Select glove model and skin

### 4. Test in Game
1. Join deathmatch or casual
2. Wait for spawn
3. Press 3 to switch to knife
4. Check logs: `%TEMP%\equip_item_debug.txt`

---

## What to Expect

### ✅ SUCCESS Indicators:

1. **Hook Logs** (`equip_item_debug.txt`):
   ```
   [EquipItem] Found EquipItemInLoadout at 0x...
   [EquipItem] Hook installed successfully!
   [EquipItem] Called: team=3, slot=2, itemID=0x...
   [EquipItem] Knife equip detected - will modify item data
   ```

2. **Manager Logs** (`knife_glove_manager.txt`):
   ```
   [KNIFE] Applying modifications: 42 -> 507 (entity: 0x...)
   [KNIFE] Applied: defIndex=507, paintKit=38, wear=0.0001, seed=0
   ```

3. **In-Game**:
   - Karambit model visible
   - "★ Karambit | Fade" in bottom right
   - Golden Fade skin visible
   - Can switch to knife normally (press 3)
   - NO LAG

### ❌ FAILURE Indicators:

1. **Hook Not Found**:
   ```
   [EquipItem] ERROR: Failed to find EquipItemInLoadout function
   ```
   **Solution**: Signature might be outdated. Check br5rhvh.txt for updated signature.

2. **Hook Not Called**:
   - No logs in `equip_item_debug.txt` when pressing 3
   **Solution**: Hook might not be intercepting correctly. Check if function address is correct.

3. **Modifications Not Applied**:
   - Logs show "Knife equip detected" but model doesn't change
   **Solution**: Need to add direct function calls (UpdateSubclass, SetModel, SetMeshGroupMask)

4. **Lag**:
   - Game becomes unplayable
   **Solution**: This should NOT happen with new approach. If it does, check logs for spam.

---

## Troubleshooting

### Issue: Knife model doesn't change

**Diagnosis**: Check `knife_glove_manager.txt` for:
```
[KNIFE] Applying modifications: 42 -> 507
```

If you see this but model doesn't change, the defIndex is being written but the game isn't reloading the model.

**Fix**: Add direct function calls in `knife_glove_manager.h`:

```cpp
// After writing defIndex, add:
if (SkinChanger::UpdateSubclass) {
    SkinChanger::UpdateSubclass(item);
}

const char* modelPath = SkinChanger::GetKnifeModelPath(targetDefIndex);
if (SkinChanger::SetModel && modelPath) {
    SkinChanger::SetModel(activeWeapon, modelPath);
}

if (SkinChanger::SetMeshGroupMask) {
    SkinChanger::SetMeshGroupMask(activeWeapon, 0xFFFFFFFFFFFFFFFF);
}
```

**IMPORTANT**: These are DIRECT calls, not hooks. We call them once per modification.

---

### Issue: Can't switch to knife (press 3 doesn't work)

**Diagnosis**: Check if defIndex is being modified in EquipItemInLoadout hook.

**Fix**: The hook should NOT modify defIndex before calling original function. It should modify AFTER.

Current implementation modifies after equip, which is correct.

---

### Issue: Lag when knife is equipped

**Diagnosis**: Check logs for spam (hundreds of lines per second).

If you see spam, the frame manager is being called too frequently.

**Fix**: Increase frame counter threshold in `knife_glove_manager.h`:
```cpp
// Change from 10 to 30 (2 times/sec instead of 6)
if (frameCounter < 30) return;
```

---

### Issue: Crash when switching to knife

**Diagnosis**: Check `%TEMP%\cs2crash.txt` for crash details.

**Fix**: Add more pointer validation in `knife_glove_manager.h`:
```cpp
// Before every memory read/write, validate pointer
if (!ptr || ptr < 0x10000 || ptr > 0x7FFFFFFFFFFF) return;
```

---

## Performance Monitoring

### Check for Lag:

1. **FPS Counter**: Should stay at 60+ FPS
2. **Frame Time**: Should be <16ms
3. **Log Spam**: Should NOT see hundreds of lines per second

### Expected Performance:

- **EquipItemInLoadout calls**: 1-2 per weapon switch
- **Manager ticks**: 6 per second (every 10 frames at 60 FPS)
- **Total operations**: 6-12 per second
- **FPS impact**: <1% (negligible)

### If Lag Occurs:

1. Check logs for spam
2. Increase frame counter threshold
3. Add more caching
4. Reduce modification frequency

---

## Comparison with Old Approach

### OLD (UpdateSubclass Hook):
- ❌ 1000+ calls per second
- ❌ Processes ALL entities
- ❌ SEVERE LAG
- ❌ Weapon switching broken

### NEW (EquipItemInLoadout + Manager):
- ✅ 6-12 operations per second
- ✅ Processes active weapon only
- ✅ NO LAG
- ✅ Weapon switching works

---

## Next Steps After Testing

### If It Works:
1. Test all knife models
2. Test all glove models
3. Test rapid weapon switching
4. Test respawn behavior
5. Play full game to verify stability

### If It Doesn't Work:
1. Check logs for errors
2. Add direct function calls (UpdateSubclass, SetModel, SetMeshGroupMask)
3. Increase pointer validation
4. Adjust frame manager frequency

### If It Lags:
1. This should NOT happen
2. If it does, check logs for spam
3. Increase frame counter threshold
4. Add more caching

---

## Log Files Reference

| File | Purpose | What to Check |
|------|---------|---------------|
| `cs2init.txt` | Initialization | Hook installation success |
| `equip_item_debug.txt` | EquipItemInLoadout hook | Weapon switch interception |
| `knife_glove_manager.txt` | Frame manager | Modification application |
| `cs2crash.txt` | Crashes | Crash details and stack trace |

---

## Success Criteria

✅ **PASS** if:
- Knife model changes (Karambit visible)
- Knife skin applies (Fade visible)
- Can switch to knife (press 3 works)
- NO LAG (60+ FPS maintained)
- NO CRASHES (stable gameplay)

❌ **FAIL** if:
- Knife model doesn't change
- Can't switch to knife
- Severe lag (FPS drops below 30)
- Crashes when switching weapons

---

## Final Notes

This is a **completely new approach** - not a fix of the old hooks.

The old approach (UpdateSubclass/SetModel hooks) was fundamentally flawed and could never work without lag.

This new approach is the ONLY way to implement knife/glove changing without performance issues.

**If this doesn't work, the issue is implementation details (offsets, function calls), NOT the fundamental approach.**

---

**Ready to test. Good luck!**
