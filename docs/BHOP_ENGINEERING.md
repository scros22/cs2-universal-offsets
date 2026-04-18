# Bhop Engineering Document — 2026 Perfect Hop

## Current Implementation (bhop.h) — Problems

1. **Memory write detection**: Writes `65537` to `client.dll + jump` button address.
   Memory watchpoints / page-guard traps catch this trivially.
2. **No tick precision**: Just reads FL_ONGROUND and writes jump. No sub-tick awareness.
3. **Binary miss chance**: Random 10% miss is not how humans fail at bhop.
   Humans fail with timing variance, not random skips.
4. **No speed awareness**: Hops from standstill (kills speed), no velocity monitoring.
5. **No strafe logic**: No air-strafe assistance for speed preservation/gain.

## CS2 Movement System (2026)

### Sub-Tick Movement
CS2 uses a **sub-tick system** where player inputs are timestamped within the tick:
- Server ticks at 64 tick (competitive) or 128 tick (faceit)
- Client inputs have sub-tick timestamps for precise action timing
- A "perfect hop" means issuing +jump at the EXACT sub-tick of landing
- The game processes jump commands based on their sub-tick timestamp

### Key Offsets for Bhop
- `m_fFlags` (0x400): Bit 0 = FL_ONGROUND
- `m_vecVelocity` (in C_BasePlayerPawn): Current velocity vector
- `m_flSimulationTime` (0x3C0): Last server simulation time
- `Buttons::jump`: Jump command address in client.dll
- `m_MoveType` (via m_nActualMoveType): Movement type (walk/fly/noclip)

### Speed Mechanics
- Walking speed: ~250 u/s (no crouch)
- Max bhop speed (legit): ~285-300 u/s with good strafing
- Air acceleration: Limited by `sv_airaccelerate` (default 12)
- Speed is preserved on landing IF you don't hold W
- Strafing: Hold A/D + move mouse in that direction while airborne = speed gain

## New Architecture: SendInput-Based Perfect Hop

### Core Strategy
Instead of writing to game memory, send keyboard events via SendInput:
1. Monitor FL_ONGROUND flag (read-only)
2. When landing detected, send VK_SPACE press via `INPUT_KEYBOARD`
3. Release on next tick
4. All input goes through Windows input queue — indistinguishable from keyboard

### Landing Detection (Ultra-Precise)
```
Previous tick: FL_ONGROUND = 0 (airborne)
Current tick:  FL_ONGROUND = 1 (just landed)
→ INSTANTLY send jump input
```

Additional precision from velocity:
- Track `m_vecVelocity.z` (vertical speed)
- When z-velocity transitions from negative to ~0 = about to land
- Pre-buffer the jump input for maximum responsiveness

### Speed Preservation Logic
```
float hSpeed = sqrt(vel.x² + vel.y²);

if (hSpeed < 50.0)    → Don't hop (standing still / slow walk)
if (hSpeed > 300.0)   → Pause hopping (speed too high, suspicious)
if (onGround && !wasOnGround && hSpeed > 80.0) → HOP
```

### Strafe Assist (Optional)
When airborne, inject A/D keypresses to maintain/gain speed:
1. Read view yaw and velocity direction
2. Calculate velocity-to-view angle delta  
3. If delta > threshold, inject A/D key to correct
4. Subtle — only corrects when clearly losing speed

### Humanization Model
Instead of random miss%, use timing-based variance:
- **Perlin noise jitter**: 0-6ms delay on jump timing (mimics human reaction)
- **Speed-dependent hop rate**: Faster = slightly more likely to miss (fatigue)
- **Consecutive hop limit**: After 8-15 hops, force a small stumble (1-2 ticks late)
- **Session drift**: Max consecutive hops slowly changes through the session

### Anti-Detection Limits
- Hard speed cap at 300 u/s (server flags players above this)
- Auto-stop hopping when observed by spectators (if we can detect)
- Hop success rate target: 85-92% (pro KZ players hit ~88%)
- Never hop during freeze time or buy period

## SendInput Implementation Details

```cpp
inline void SendKeyDown(WORD vk)
{
    INPUT inp = {};
    inp.type = INPUT_KEYBOARD;
    inp.ki.wVk = vk;
    inp.ki.dwFlags = 0;  // key down
    SpoofCall::SpoofedSendInput(1, &inp, sizeof(INPUT));
}

inline void SendKeyUp(WORD vk)
{
    INPUT inp = {};
    inp.type = INPUT_KEYBOARD;
    inp.ki.wVk = vk;
    inp.ki.dwFlags = KEYEVENTF_KEYUP;
    SpoofCall::SpoofedSendInput(1, &inp, sizeof(INPUT));
}
```

### Tick State Machine
```
IDLE        → waiting for space held
AIRBORNE    → space held, in air, monitoring for landing
PREJUMP     → velocity indicates imminent landing, buffer jump
JUMPING     → jump key sent, wait for liftoff confirmation
COOLDOWN    → brief pause to prevent double-input
```

## Menu Controls
- **Enable**: Toggle
- **Key**: Bind (default: Space)
- **Strafe Assist**: Toggle (default: off)
- **Max Speed**: Slider 250-320 (default: 300)
- **Timing Jitter**: Slider 0-10ms (default: 4ms)
- **Hop Limit**: Slider 5-30 (default: 15)
