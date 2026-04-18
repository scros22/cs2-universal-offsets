# VACNet Analysis — What Gets You Banned

## User's Known Ban Thresholds (April 2026 Testing)
- Humanization < 50 → BANNED
- Smoothing < 43 → BANNED  
- FOV > 2.3 → BANNED
- Any of these 3 conditions alone triggers conviction

## How VACNet Works (Server-Side Neural Network)

### Data Collection
- Records **pitch/yaw angular velocity** 0.5s BEFORE each shot and 0.25s AFTER
- Creates **"atoms"** per shot: velocity profile + weapon + distance + hit result
- Collects **140 atoms** from an **8-round window**
- Feeds into deep neural network → outputs conviction probability (80-95% range)

### What VACNet Measures Per Atom
1. **Angular acceleration curve** — how quickly aim starts moving
2. **Peak angular velocity** — maximum speed of crosshair travel
3. **Deceleration profile** — how aim slows approaching target
4. **Time-to-target** — reaction time from target visibility to first movement
5. **Overshoot/undershoot** — correction behavior after initial flick
6. **Post-shot stability** — micro-movement after firing
7. **Statistical consistency** — do all atoms look machine-identical?

### VACNet Red Flags
| Signal | Description | Our Status |
|--------|-------------|------------|
| Step velocity | Instant angular velocity change (0 → max) | ✅ Fixed (bell-curve) |
| Perfect lock | Zero angular movement while on target | ✅ Fixed (tremor) |
| Identical profiles | All flicks have same timing signature | ✅ Fixed (per-engagement randomization) |
| Superhuman reaction | < 100ms from visibility to first movement | ✅ Fixed (PHASE_REACTING 80-250ms) |
| HS% anomaly | > 55% headshot rate over 8 rounds | ✅ Governor caps at 55% (→ lowering to 48%) |
| Snap-back | Aim instantly returns to pre-aim position | N/A (mouse input, no angle writes) |
| FOV tunnel | All kills within identical tiny FOV cone | ⚠️ Need weapon-based FOV variation |
| Speed uniformity | All flick speeds identical regardless of distance | ⚠️ Need range-based scaling |

## Current Aimbot Config (Defaults)
```
fov            = 2.0       — DANGER: session jitter +0.2 = 2.2 (close to 2.3 ban line)
smoothing      = 40.0      — DANGER: session jitter -3 = 37 (below 43 ban line!)
humanization   = 0.55      — SAFE: session jitter -0.05 = 0.50 (at floor)
hsCapPercent   = 55.0      — TOO HIGH: pro average is 42-52%
```

## Safe Config (Post-Hardening)
```
fov            = 1.8       — max with jitter: 2.0 (well below 2.3)
smoothing      = 55.0      — min with jitter: 52.0 (well above 43)
humanization   = 0.60      — min with jitter: 0.55 (comfortable buffer)
hsCapPercent   = 48.0      — within pro distribution
```

## New Anti-VACNet Features Planned

### 1. Weapon-Aware Profiles
Different weapons have different natural aim patterns:
- **AWP/Scout**: Higher smoothing (70+), smaller FOV (1.2), longer reaction delay
- **AK/M4/Rifle**: Standard settings (smoothing 55, FOV 1.8)
- **Pistol**: More aggressive humanization (0.70), wider FOV (2.0), faster reactions
- **SMG**: Slightly lower smoothing (50), body preference

### 2. Range-Based Aggression
- **Close (< 10m)**: +20% smoothing (panic spray looks human)
- **Medium (10-40m)**: Standard settings
- **Long (> 40m)**: -15% smoothing (long-range flicks are naturally faster)

### 3. Mid-Session Drift (Fatigue Simulation)
Every ~5 minutes, slowly re-roll jitter values with 70% overlap:
- Simulates player getting tired, warming up, losing focus
- Makes 30-minute statistical profile look different from first/last 5 minutes

### 4. Shot Timing Variance
1-3 tick delay between PHASE_LOCKED and actual commitment to fire
- Humans don't pull trigger the instant crosshair touches bone
- Adds 15-50ms natural delay to shot timing
