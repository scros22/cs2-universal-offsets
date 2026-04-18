# Testing Knife & Glove Changer

## Build Status
✅ **Compiled successfully** - Ready for testing

## What Was Changed

### Enhanced Debugging
- Added extensive logging to OutputDebugString (view with DebugView++)
- Logs every 100 ticks to avoid spam
- Shows which method (m_EconGloves vs m_hMyWearables) is being used

### Dual Glove Detection Method
The code now tries TWO ways to find gloves:

1. **Method 1: m_EconGloves** (offset 0x1890)
   - Direct pointer to glove entity
   - This is what unix.solutions uses

2. **Method 2: m_hMyWearables** (offset 0x1350)
   - Vector of wearable entity handles
   - Fallback if m_EconGloves is NULL

### Knife Changes
- Added bounds checking (cfg.knifeModel < kKnifeCount)
- Added debug logging every 100 ticks
- Shows current vs target defIndex

## How to Test

### 1. Download DebugView++
- Get it from: https://github.com/CobaltFusion/DebugViewPP/releases
- This will show the debug logs from the cheat

### 2. Start DebugView++
- Run DebugView++.exe
- Keep it open in the background

### 3. Inject the Cheat
```
1. Start CS2 (get to main menu)
2. Run x64\Lucid.exe
3. Wait for "Injection successful" message
```

### 4. Enable Skinchanger
```
1. Press INSERT to open menu
2. Go to "Skinchanger" tab
3. Enable skinchanger checkbox
```

### 5. Test Knives
```
1. In menu, under "KNIFE CHANGER":
   - Enable checkbox
   - Select knife model (e.g., "Karambit" = index 7)
   - Set Paint Kit (e.g., 44 for Case Hardened)
   - Set Seed (e.g., 387 for Blue Gem)
   - Set Wear (e.g., 0.0001 for Factory New)

2. Join a casual/deathmatch game
3. Equip knife (press 3)
4. Check DebugView++ for logs like:
   [KNIFE] Current: 42, Target: 507, Model: 7
   [KNIFE] Wrote defIndex
```

### 6. Test Gloves
```
1. In menu, under "GLOVE CHANGER":
   - Enable checkbox
   - Select glove model (e.g., "Sport Gloves" = index 1)
   - Set Paint Kit (e.g., 10053 for Spearmint)
   - Set Wear (e.g., 0.0001)

2. Already in game (or join if not)
3. Check DebugView++ for logs like:
   [GLOVE] m_EconGloves: 0x... Target DefIndex: 5027
   [GLOVE] Current DefIndex: 0
   [GLOVE] Wrote via m_EconGloves
   
   OR if m_EconGloves is NULL:
   [GLOVE] m_hMyWearables size: 1, data: 0x...
   [GLOVE] Wearable handle: 0x..., entity: 0x...
   [GLOVE] Wrote via m_hMyWearables
```

## Expected Debug Output

### If Knives Work:
```
[KNIFE] Current: 42, Target: 507, Model: 7
[KNIFE] Wrote defIndex
```
- Current 42 = CT default knife
- Target 507 = Karambit
- You should see Karambit model in-game

### If Gloves Work (Method 1):
```
[GLOVE] m_EconGloves: 0x1A2B3C4D5E6F, Target DefIndex: 5027
[GLOVE] Current DefIndex: 0
[GLOVE] Wrote via m_EconGloves
```
- Non-zero m_EconGloves address = glove entity exists
- Should see Sport Gloves in first-person view

### If Gloves Work (Method 2):
```
[GLOVE] m_EconGloves: 0x0, Target DefIndex: 5027
[GLOVE] m_hMyWearables size: 1, data: 0x1A2B3C4D
[GLOVE] Wearable handle: 0x12345, entity: 0x1A2B3C4D5E6F
[GLOVE] Wrote via m_hMyWearables
```
- m_EconGloves is NULL, fallback to wearables vector
- Should still see gloves

### If Gloves DON'T Work:
```
[GLOVE] m_EconGloves: 0x0, Target DefIndex: 5027
[GLOVE] m_hMyWearables size: 0, data: 0x0
```
- Both methods failed to find glove entity
- This means offsets are wrong OR gloves don't exist yet

## Troubleshooting

### Knives Not Changing
**Check DebugView++ logs:**
- If you see `[KNIFE] Wrote defIndex` but model doesn't change:
  - Offset might be wrong
  - Try different knife models
  - Check if weapon skins work (if yes, offsets are correct)

### Gloves Not Appearing
**Check DebugView++ logs:**

**If you see:**
```
[GLOVE] m_EconGloves: 0x0
[GLOVE] m_hMyWearables size: 0
```
**This means:**
- Glove entity doesn't exist yet
- Might need to be in-game longer
- Might need to die/respawn first
- Offsets could be wrong

**If you see:**
```
[GLOVE] Wrote via m_EconGloves
```
**But gloves don't appear:**
- DefIndex write succeeded but not visible
- Might need to call SetModel function
- Might need different approach (create wearable entity)

### Weapon Skins Work But Not Knives/Gloves
- This confirms offsets are correct
- Knife/glove changing needs additional steps
- May need to hook SetModel or create entities

## Next Steps If Still Not Working

### 1. Verify Offsets
Check if these offsets are correct in latest CS2 build:
- m_EconGloves = 0x1890
- m_bNeedToReApplyGloves = 0x188D
- m_hMyWearables = 0x1350

### 2. Try Creating Wearable Entity
If glove entity doesn't exist, we may need to:
- Use CreateEntityByClassName signature
- Create "item_wearable" entity
- Attach it to player

### 3. Try SetModel Function
If defIndex writes don't change model:
- Use SetModel signature from br5rhvh.txt
- Call it with model path string
- Example: "weapons/models/knife/karambit/weapon_knife_karambit.vmdl"

## Share Debug Logs

**Please copy the DebugView++ output and share it so I can see:**
1. Whether glove entity exists (m_EconGloves address)
2. Whether wearables vector has entries
3. Whether defIndex writes are happening
4. Any error patterns

This will help identify the exact issue!
