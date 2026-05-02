#pragma once

// ---------------------------------------------------------------
// Kill Sound â€” replaces Valve's headshot/kill ding with a custom
// short metallic "ding".
//
// Hooks the per-kill AttackerFeedback emitter in client.dll
// (sub_180849CE0 on build 14155). That single function is the
// dispatch point for all four "Player.Death*.AttackerFeedback"
// sound events â€” head/body Ã— armor/no-armor. By short-circuiting
// it (return 0, skip the engine sound emit) and triggering our
// own PlaySoundA from a synthesized in-memory WAV, we get:
//
//   * Valve's headshot kill ding muted (the iconic CS2 ding that
//     plays on every HS kill â€” server-side telemetry doesn't see
//     this, so it's purely a client-side cosmetic change).
//   * Custom satisfying ding on EVERY confirmed kill (head OR
//     body), driven by the engine's own kill detection flow â€”
//     way more reliable than hooking our aimbot's lock state.
//
// WAV is synthesized once at Setup() (8kHz two-tone metallic chirp,
// ~140ms total, 16-bit PCM mono 22050Hz, ~6KB). Lives on the heap
// for the cheat's lifetime so PlaySoundA can stream from it. Heap
// pages look like normal allocations, no PAGE_EXECUTE involvement.
// ---------------------------------------------------------------

#include <Windows.h>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <atomic>
#include "../../core/memory.h"
#include "../../core/signatures.h"
#include "../../vendor/minhook/include/MinHook.h"

namespace KillSound
{
    struct Config
    {
        bool  enabled = true;          // play our custom ding on confirmed kill
        bool  muteValve = true;         // skip Valve's HS/body kill DEATH ding (works even with enabled=false)
        bool  muteValveHits = true;     // skip Valve's per-HIT feedback (HS dink AND body thud)
        bool  logSounds = true;         // capture every emitted sound name into a ring buffer (diagnostic)
        float volume = 0.55f;          // 0..1 â€” playback amplitude scalar baked into WAV
    };
    inline Config cfg;

    // Hook plumbing
    using KillFeedbackFn  = __int64(__fastcall*)(__int64, __int64);
    using DmgFeedbackFn   = void   (__fastcall*)(__int64, void*, __int64);
    using HitGroupGetFn   = __int64(__fastcall*)(__int64);
    using EmitSoundFn     = __int64(__fastcall*)(__int64, int, int, void*);
    // soundsystem.dll!CSosOperatorSystem::StartSoundEvent (sub_1801B7AD0)
    // The unified queued entrypoint. R8 points to a StartSoundEventInfo_t:
    //   +0  const char* name  (may be null on by-handle path!)
    //   +16 uint32_t    hash  (always populated, MurmurHash2 of name)
    using StartSoundEventFn = uint32_t* (__fastcall*)(void* self, uint32_t* outGuid, void* info);

    inline KillFeedbackFn oKillFeedback     = nullptr;
    inline DmgFeedbackFn  oDmgFeedback      = nullptr;
    inline EmitSoundFn    oEmitSound        = nullptr;
    inline StartSoundEventFn oStartSoundEvent = nullptr;
    inline HitGroupGetFn  pGetHitGroup      = nullptr; // not hooked, just called

    inline void* pKillFeedbackTarget = nullptr;
    inline void* pDmgFeedbackTarget  = nullptr;
    inline void* pEmitSoundTarget    = nullptr;
    inline void* pStartSoundEventTarget = nullptr;
    inline bool  hooked       = false; // kill (death) hook
    inline bool  dmgHooked    = false; // damage (per-hit) hook
    inline bool  emitHooked   = false; // universal sound emit hook
    inline bool  sosHooked    = false; // soundsystem.dll StartSoundEvent hook (THE HS dink killer)
    inline bool  dmgPatched   = false; // hard NOP-patch on sub_18081ED00 prologue
    inline uint8_t g_dmgPatchOriginal[3] = {}; // original 3 bytes for restore

    // Synthesized WAV buffer (heap-allocated, owned for cheat lifetime)
    inline uint8_t* g_wav = nullptr;
    inline int      g_wavSize = 0;

    // Stat counter (diagnostics â€” number of kill events seen)
    inline std::atomic<uint32_t> g_kills{ 0 };
    // Diagnostic: per-hit damage feedback dispatcher invocations.
    // If this stays at 0 while shooting players, the dmg hook didn't
    // install (sig miss) OR Valve is dispatching the HS dink through
    // a code path other than sub_18081ED00.
    inline std::atomic<uint32_t> g_hitsMuted{ 0 };
    inline std::atomic<uint32_t> g_hitsPassed{ 0 };
    // Diagnostic: universal sound-emit dispatcher invocations.
    // emitMuted increments every time we drop an HS-related event
    // by name match in the sound dispatcher.
    inline std::atomic<uint32_t> g_emitMuted{ 0 };
    // Diagnostic: soundsystem.dll-side mute count (the real path).
    inline std::atomic<uint32_t> g_sosMuted{ 0 };

    // ---- Sound name capture ring buffer (diagnostic) ----
    // When cfg.logSounds is on, every event name passing through
    // EmitSoundByHandle is written to this ring. The menu displays
    // the most-recent entries so we can see exactly which event
    // names are emitted around HS / kills, then add them to the
    // mute list. 32 slots * 64 chars = 2KB total.
    static constexpr int kSoundLogSlots = 32;
    static constexpr int kSoundLogLen   = 64;
    inline char  g_soundLog[kSoundLogSlots][kSoundLogLen] = {};
    inline std::atomic<uint32_t> g_soundLogHead{ 0 };

    inline void LogSoundName(const char* name)
    {
        if (!name) return;
        uint32_t idx = g_soundLogHead.fetch_add(1, std::memory_order_relaxed) % kSoundLogSlots;
        // strncpy is fine here â€” buffer is fixed-size, we want truncation.
        size_t i = 0;
        for (; i < kSoundLogLen - 1 && name[i]; ++i) g_soundLog[idx][i] = name[i];
        g_soundLog[idx][i] = '\0';

        // Also append to a file on disk so the user can just open it
        // instead of needing the in-game menu open during testing.
        // Path: %TEMP%\lucid_sounds.log
        static HANDLE hLog = INVALID_HANDLE_VALUE;
        static bool   triedOpen = false;
        if (!triedOpen)
        {
            triedOpen = true;
            char tmp[MAX_PATH];
            DWORD n = GetTempPathA(MAX_PATH, tmp);
            if (n > 0 && n < MAX_PATH - 32)
            {
                strcat_s(tmp, MAX_PATH, "lucid_sounds.log");
                hLog = CreateFileA(tmp, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                   nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
                if (hLog != INVALID_HANDLE_VALUE)
                {
                    const char* hdr = "\r\n=== lucid sound log session ===\r\n";
                    DWORD w; WriteFile(hLog, hdr, (DWORD)strlen(hdr), &w, nullptr);
                }
            }
        }
        if (hLog != INVALID_HANDLE_VALUE)
        {
            char line[kSoundLogLen + 4];
            int  ll = 0;
            for (; ll < kSoundLogLen - 1 && name[ll]; ++ll) line[ll] = name[ll];
            line[ll++] = '\r'; line[ll++] = '\n';
            DWORD w; WriteFile(hLog, line, (DWORD)ll, &w, nullptr);
            FlushFileBuffers(hLog);
        }
    }

    // -----------------------------------------------------------
    // Build a short metallic "ding": two superimposed sine tones
    // (1200Hz fundamental + 1800Hz overtone) with a fast attack
    // and exponential decay. ~140ms total. Mono, 16-bit PCM,
    // 22050Hz. Output is a fully-formed RIFF WAV in memory.
    // -----------------------------------------------------------
    inline bool BuildDingWav()
    {
        constexpr int sampleRate     = 22050;
        constexpr int bitsPerSample  = 16;
        constexpr int channels       = 1;
        constexpr int durationMs     = 140;
        constexpr float freq1        = 1200.f;
        constexpr float freq2        = 1800.f;
        const     float vol          = cfg.volume;

        const int totalSamples = sampleRate * durationMs / 1000;
        const int dataSize     = totalSamples * (bitsPerSample / 8);
        const int wavSize      = 44 + dataSize;

        auto* wav = (uint8_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, wavSize);
        if (!wav) return false;

        // RIFF header
        memcpy(wav, "RIFF", 4);
        *(uint32_t*)(wav + 4)  = wavSize - 8;
        memcpy(wav + 8, "WAVE", 4);
        memcpy(wav + 12, "fmt ", 4);
        *(uint32_t*)(wav + 16) = 16;
        *(uint16_t*)(wav + 20) = 1; // PCM
        *(uint16_t*)(wav + 22) = channels;
        *(uint32_t*)(wav + 24) = sampleRate;
        *(uint32_t*)(wav + 28) = sampleRate * channels * bitsPerSample / 8;
        *(uint16_t*)(wav + 32) = (uint16_t)(channels * bitsPerSample / 8);
        *(uint16_t*)(wav + 34) = bitsPerSample;
        memcpy(wav + 36, "data", 4);
        *(uint32_t*)(wav + 40) = dataSize;

        auto* samples = (int16_t*)(wav + 44);
        const float twoPi = 6.2831853f;

        // Decay constants â€” sharp attack (~3ms), exponential tail
        const float attackS = 0.003f;
        const float decayK  = 18.f;  // larger = faster decay

        for (int i = 0; i < totalSamples; ++i)
        {
            float t = (float)i / sampleRate;

            // Envelope: fast linear attack, exponential decay
            float env;
            if (t < attackS) env = t / attackS;
            else             env = expf(-decayK * (t - attackS));

            // Two-tone sum, overtone slightly quieter
            float s = sinf(twoPi * freq1 * t) * 0.65f
                    + sinf(twoPi * freq2 * t) * 0.35f;

            float val = s * env * vol;
            // Soft clip
            if (val >  0.95f) val =  0.95f;
            if (val < -0.95f) val = -0.95f;

            samples[i] = (int16_t)(val * 32767.f);
        }

        g_wav     = wav;
        g_wavSize = wavSize;
        return true;
    }

    // -----------------------------------------------------------
    // PlayDing â€” fire-and-forget. winmm queues to mixer, returns
    // immediately. Cheap enough to call from the hook.
    // -----------------------------------------------------------
    inline void PlayDing()
    {
        if (!g_wav) return;
        // SND_MEMORY: read WAV from buffer
        // SND_ASYNC : return immediately
        // SND_NODEFAULT: don't play the system "ding" if buffer is bad
        PlaySoundA((LPCSTR)g_wav, nullptr, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
    }

    // -----------------------------------------------------------
    // Detour â€” runs on every confirmed kill (the engine calls this
    // from CCSPlayer_Pawn damage/death dispatch). We:
    //   1. bump the kill counter
    //   2. play our ding (cfg.enabled gate)
    //   3. either skip Valve's emit (cfg.muteValve) or pass through
    //
    // The two gates are INDEPENDENT â€” muteValve no longer requires
    // enabled=true. This lets users pick:
    //   muteValve=1 enabled=1  â†’ our ding only (default â€œcustomâ€ feel)
    //   muteValve=1 enabled=0  â†’ silent kills (no ding at all)
    //   muteValve=0 enabled=1  â†’ our ding stacked on top of Valve's
    //   muteValve=0 enabled=0  â†’ stock Valve behavior (hook is no-op)
    //
    // Signature is __int64(__int64, __int64). The decompiled
    // function returns either an early result int64 or the inner
    // sound-emit return. Returning 0 from the early-skip path is
    // safe (matches the natural early-bailout cases in the original).
    // -----------------------------------------------------------
    inline __int64 __fastcall hkKillFeedback(__int64 a1, __int64 a2)
    {
        __try
        {
            g_kills.fetch_add(1, std::memory_order_relaxed);
            if (cfg.enabled) PlayDing();
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {}

        if (cfg.muteValve)
            return 0; // skip Valve's sound event emit entirely

        // Pass through â€” Valve's ding still plays
        if (oKillFeedback)
            return oKillFeedback(a1, a2);
        return 0;
    }

    // -----------------------------------------------------------
    // Per-HIT damage feedback detour â€” sub_18081ED00 dispatches:
    //   Player.DamageHeadShot[Armor].AttackerFeedback   (the iconic dink)
    //   Player.DamageBody[Armor].AttackerFeedback       (body hit thud)
    //   Player.DamageBody[Armor].Knife.AttackerFeedback (knife hit)
    //   Player.DamageKevlar / Flesh.BulletImpact        (low-dmg fallbacks)
    //   Player.BurnDamage[Kevlar]                       (molotov)
    //
    // We replicate the engine's HS gate from the function prologue:
    //   if ( (*(BYTE*)(*a2 + 76) & 2) && *(QWORD*)(*a2 + 104) )
    //       v7 = sub_180A163A0(a1);     // pGetHitGroup, returns hitgroup
    //   if v7 == 1  -> headshot branch  (the dink we want gone)
    //
    // When cfg.muteValveHSDmg is on AND the call is going to take the
    // HS branch, we return immediately â€” NOTHING else in this function
    // runs, so the body/burn/etc. dings are unaffected. Body hit-marker
    // sounds, knife thuds, molotov burn ack: ALL preserved.
    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // Per-HIT damage feedback detour â€” sub_18081ED00 dispatches
    // EVERY hit-feedback sound the local attacker hears:
    //   Player.DamageHeadShot[Armor].AttackerFeedback   (HS dink)
    //   Player.DamageBody[Armor].AttackerFeedback       (body thud)
    //   Player.DamageBody[Armor].Knife.AttackerFeedback (knife thud)
    //   Player.DamageKevlar / Flesh.BulletImpact        (low-dmg)
    //   Player.BurnDamage[Kevlar]                       (molotov)
    //
    // Footsteps, gunshots, reloads, voice lines, music, etc. all
    // emit through completely different code paths â€” nuking this
    // single function only kills the hit-confirmation chirps.
    //
    // We early-return when cfg.muteValveHits is on. None of the
    // function's other side effects (decals, blood, hit markers in
    // engine state) live in here â€” it is purely the sound dispatcher.
    // -----------------------------------------------------------
    inline void __fastcall hkDmgFeedback(__int64 a1, void* a2, __int64 a3)
    {
        if (cfg.muteValveHits)
        {
            g_hitsMuted.fetch_add(1, std::memory_order_relaxed);
            return;
        }
        g_hitsPassed.fetch_add(1, std::memory_order_relaxed);
        if (oDmgFeedback)
            oDmgFeedback(a1, a2, a3);
    }

    // -----------------------------------------------------------
    // Universal sound-emit detour â€” sub_180B62270 (EmitSoundByHandle)
    // is the single funnel EVERY in-game sound goes through. The 4th
    // argument's first qword is a pointer to the event-name C-string
    // ("Player.DamageHeadShot.AttackerFeedback", "UI.DeathNotice",
    // "Weapon_AK47.Single", etc.). When cfg.muteValveHits is on we
    // string-match against known HS-related events and drop them
    // before they reach the engine. Everything else (footsteps,
    // gunshots, voice, music, body sounds) passes through untouched.
    //
    // Name list is intentionally narrow â€” we ONLY drop the headshot
    // chirp variants and the killfeed ding. Body damage feedback is
    // handled by the dmg hook above; this is the safety net for any
    // path we missed.
    // -----------------------------------------------------------
    inline bool IsMutedSoundName(const char* name)
    {
        if (!name) return false;
        // Cheap tolerant substring check. Engine uses fixed casing
        // so plain strstr suffices and avoids per-emit allocations.
        // Cover every known HS/kill/death-ack variant we've seen
        // referenced in client.dll strings, plus likely engine-side
        // names since the actual emit may live in soundsystem.dll.
        if (strstr(name, "HeadShot"))         return true;
        if (strstr(name, "Headshot"))         return true;
        if (strstr(name, "headshot"))         return true;
        if (strstr(name, "DeathNotice"))      return true;
        if (strstr(name, "Player.Death"))     return true;
        if (strstr(name, "Player.Damage"))    return true; // per-hit acks (HS + body)
        if (strstr(name, "AttackerFeedback")) return true; // catches any *.AttackerFeedback variant
        if (strstr(name, "DamageHelmet"))     return true;
        return false;
    }

    inline __int64 __fastcall hkEmitSound(__int64 a1, int a2, int a3, void* a4)
    {
        const char* name = nullptr;
        if (a4)
        {
            __try { name = *reinterpret_cast<const char**>(a4); }
            __except (EXCEPTION_EXECUTE_HANDLER) { name = nullptr; }
        }
        if (cfg.logSounds && name)
            LogSoundName(name);
        if (cfg.muteValveHits && IsMutedSoundName(name))
        {
            g_emitMuted.fetch_add(1, std::memory_order_relaxed);
            return a1; // matches the function's normal return value
        }
        if (oEmitSound)
            return oEmitSound(a1, a2, a3, a4);
        return a1;
    }

    // -----------------------------------------------------------
    // soundsystem.dll!CSosOperatorSystem::StartSoundEvent detour
    // (sub_1801B7AD0 â€” the unified queued entrypoint).
    //
    // Both the by-name (vtable[13]) and by-handle (vtable[11], used
    // by the HS dink) paths converge here. The info struct's name
    // pointer at +0 may be null on the by-handle path, but the hash
    // at +16 (MurmurHash2 of the original sound name) is always set.
    //
    // We filter on either: try name first (cheap & readable for the
    // log), fall back to hash comparison against precomputed bans.
    // -----------------------------------------------------------
    static constexpr uint32_t kMurmurSeed = 0x31415926; // Source 2 SOS hash seed

    inline uint32_t Murmur2(const char* data, size_t len, uint32_t seed = kMurmurSeed)
    {
        const uint32_t m = 0x5BD1E995u;
        const int      r = 24;
        uint32_t h = seed ^ static_cast<uint32_t>(len);
        while (len >= 4)
        {
            uint32_t k;
            std::memcpy(&k, data, 4);
            k *= m; k ^= k >> r; k *= m;
            h *= m; h ^= k;
            data += 4; len -= 4;
        }
        switch (len)
        {
            case 3: h ^= static_cast<uint8_t>(data[2]) << 16; [[fallthrough]];
            case 2: h ^= static_cast<uint8_t>(data[1]) << 8;  [[fallthrough]];
            case 1: h ^= static_cast<uint8_t>(data[0]);       h *= m;
        }
        h ^= h >> 13; h *= m; h ^= h >> 15;
        return h;
    }
    inline uint32_t Murmur2(const char* s) { return Murmur2(s, std::strlen(s)); }

    // Precomputed hashes of every HS-related sound event we want to mute.
    // Built once at Setup(). Lookup is linear since the list is tiny.
    inline uint32_t g_banHashes[16] = {};
    inline int      g_banHashCount  = 0;

    inline void BuildBanHashList()
    {
        static const char* kBanned[] = {
            "Player.DamageHeadShot.AttackerFeedback",
            "Player.DamageHeadShotArmor.AttackerFeedback",
            "Player.DamageBody.AttackerFeedback",
            "Player.DamageBodyArmor.AttackerFeedback",
            "Player.DamageBody.Knife.AttackerFeedback",
            "Player.DamageBodyArmor.Knife.AttackerFeedback",
            "Player.DeathHeadShot.AttackerFeedback",
            "Player.DeathHeadShotArmor.AttackerFeedback",
            "Player.DeathBody.AttackerFeedback",
            "Player.DeathBodyArmor.AttackerFeedback",
        };
        g_banHashCount = 0;
        for (auto* s : kBanned)
        {
            if (g_banHashCount >= (int)(sizeof(g_banHashes) / sizeof(g_banHashes[0]))) break;
            g_banHashes[g_banHashCount++] = Murmur2(s);
        }
    }

    inline bool IsBannedHash(uint32_t h)
    {
        for (int i = 0; i < g_banHashCount; ++i)
            if (g_banHashes[i] == h) return true;
        return false;
    }

    inline uint32_t* __fastcall hkStartSoundEvent(void* self, uint32_t* outGuid, void* info)
    {
        const char* name = nullptr;
        uint32_t    hash = 0;
        if (info)
        {
            __try
            {
                name = *reinterpret_cast<const char**>(info);
                hash = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(info) + 16);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) { name = nullptr; hash = 0; }
        }

        if (cfg.logSounds)
        {
            // Log either the name (preferred) or a hex hash so we can
            // see by-handle paths even when the name pointer is null.
            if (name)
            {
                LogSoundName(name);
            }
            else if (hash)
            {
                // Dump first 48 bytes of info struct as hex + ascii so we
                // can spot the real name pointer / string in by-handle path.
                char buf[256];
                uint8_t* p = reinterpret_cast<uint8_t*>(info);
                __try {
                    _snprintf_s(buf, sizeof(buf), _TRUNCATE,
                        "<h:%08X> q0=%016llX q1=%016llX q2=%016llX q3=%016llX q4=%016llX q5=%016llX",
                        hash,
                        *(uint64_t*)(p + 0),  *(uint64_t*)(p + 8),
                        *(uint64_t*)(p + 16), *(uint64_t*)(p + 24),
                        *(uint64_t*)(p + 32), *(uint64_t*)(p + 40));
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    _snprintf_s(buf, sizeof(buf), _TRUNCATE, "<hash:%08X>", hash);
                }
                LogSoundName(buf);
                // For each qword that looks like a pointer into a readable region,
                // try to read the first 32 chars as a C string and log if printable.
                for (int off = 0; off <= 40; off += 8) {
                    char* candidate = nullptr;
                    __try { candidate = *reinterpret_cast<char**>(p + off); }
                    __except (EXCEPTION_EXECUTE_HANDLER) { candidate = nullptr; }
                    if (!candidate) continue;
                    // crude pointer-validity check
                    if (reinterpret_cast<uintptr_t>(candidate) < 0x10000) continue;
                    char str[64]; bool ok = false; int printable = 0;
                    __try {
                        for (int i = 0; i < 63; ++i) {
                            char c = candidate[i];
                            str[i] = c;
                            if (c == 0) { str[i] = 0; ok = (i > 2); break; }
                            if (c >= 0x20 && c < 0x7F) ++printable;
                            else { str[i] = 0; ok = false; break; }
                        }
                        str[63] = 0;
                    } __except (EXCEPTION_EXECUTE_HANDLER) { ok = false; }
                    if (ok && printable >= 4) {
                        char line[128];
                        _snprintf_s(line, sizeof(line), _TRUNCATE, "  +%d -> %s", off, str);
                        LogSoundName(line);
                    }
                }
            }
        }

        if (cfg.muteValveHits)
        {
            bool ban = false;
            if (name && IsMutedSoundName(name)) ban = true;
            if (!ban && hash && IsBannedHash(hash)) ban = true;
            if (ban)
            {
                g_sosMuted.fetch_add(1, std::memory_order_relaxed);
                if (outGuid) *outGuid = 0;
                return outGuid;
            }
        }

        if (oStartSoundEvent)
            return oStartSoundEvent(self, outGuid, info);
        if (outGuid) *outGuid = 0;
        return outGuid;
    }

    // -----------------------------------------------------------
    // Setup â€” sigscan + MinHook install. Idempotent.
    // -----------------------------------------------------------
    inline bool Setup()
    {
        if (hooked) return true;

        if (!BuildDingWav())
            return false;

        BuildBanHashList();

        __try
        {
            uintptr_t addr = Mem::FindPattern(L"client.dll", Signatures::KillFeedbackEmitter);
            if (!addr) return false;

            pKillFeedbackTarget = reinterpret_cast<void*>(addr);

            MH_STATUS st = MH_CreateHook(
                pKillFeedbackTarget,
                reinterpret_cast<void*>(&hkKillFeedback),
                reinterpret_cast<void**>(&oKillFeedback));
            if (st != MH_OK && st != MH_ERROR_ALREADY_CREATED)
                return false;

            st = MH_EnableHook(pKillFeedbackTarget);
            hooked = (st == MH_OK || st == MH_ERROR_ENABLED);

            // ---- Damage feedback hook (per-hit dink/thud killer) ----
            // Independent of `hooked` so a sig-miss on either doesn't
            // disable the other.
            uintptr_t dmgAddr = Mem::FindPattern(L"client.dll", Signatures::DamageFeedbackEmitter);
            if (dmgAddr)
            {
                pDmgFeedbackTarget = reinterpret_cast<void*>(dmgAddr);
                MH_STATUS dst = MH_CreateHook(
                    pDmgFeedbackTarget,
                    reinterpret_cast<void*>(&hkDmgFeedback),
                    reinterpret_cast<void**>(&oDmgFeedback));
                if (dst == MH_OK || dst == MH_ERROR_ALREADY_CREATED)
                {
                    MH_STATUS est = MH_EnableHook(pDmgFeedbackTarget);
                    dmgHooked = (est == MH_OK || est == MH_ERROR_ENABLED);
                }
            }

            // ---- Universal EmitSoundByHandle hook (name-filter) ----
            // Catches HS dink leaks from killfeed UI, victim-side
            // death events, and any other path the dmg hook misses.
            uintptr_t emitAddr = Mem::FindPattern(L"client.dll", Signatures::EmitSoundByHandle);
            if (emitAddr)
            {
                pEmitSoundTarget = reinterpret_cast<void*>(emitAddr);
                MH_STATUS est = MH_CreateHook(
                    pEmitSoundTarget,
                    reinterpret_cast<void*>(&hkEmitSound),
                    reinterpret_cast<void**>(&oEmitSound));
                if (est == MH_OK || est == MH_ERROR_ALREADY_CREATED)
                {
                    MH_STATUS een = MH_EnableHook(pEmitSoundTarget);
                    emitHooked = (een == MH_OK || een == MH_ERROR_ENABLED);
                }
            }

            // ---- Hard backstop: NOP-patch sub_18081ED00 prologue ----
            // Only used when MinHook FAILED to install the dmg detour
            // (signature drift, hook collision, etc.). Replacing the
            // first 3 bytes with `33 C0 C3` (xor eax,eax; ret) makes
            // the function unconditionally bail without emitting any
            // sound. If the hook IS installed we skip this since the
            // hook already does the same job cleanly via trampoline.
            if (cfg.muteValveHits && pDmgFeedbackTarget && !dmgHooked)
            {
                DWORD oldProt = 0;
                if (VirtualProtect(pDmgFeedbackTarget, 8, PAGE_EXECUTE_READWRITE, &oldProt))
                {
                    auto* p = reinterpret_cast<uint8_t*>(pDmgFeedbackTarget);
                    g_dmgPatchOriginal[0] = p[0];
                    g_dmgPatchOriginal[1] = p[1];
                    g_dmgPatchOriginal[2] = p[2];
                    p[0] = 0x33; p[1] = 0xC0; p[2] = 0xC3; // xor eax,eax; ret
                    DWORD tmp; VirtualProtect(pDmgFeedbackTarget, 8, oldProt, &tmp);
                    FlushInstructionCache(GetCurrentProcess(), pDmgFeedbackTarget, 8);
                    dmgPatched = true;
                }
            }

            // ---- soundsystem.dll!CSosOperatorSystem::StartSoundEvent ----
            // The REAL HS dink chokepoint. client.dll only forwards the
            // sound name to soundsystem.dll; this is where the audio is
            // actually scheduled. Hooking here catches every named event
            // regardless of which DLL emitted it.
            //
            // soundsystem.dll may not be loaded at the very instant our
            // DLL injects (it's load-on-first-use in some boot paths)
            // â€” if FindPattern returns 0 we silently skip; the kill
            // and dmg hooks above still install. The user can re-enable
            // the feature later when the module is present.
            HMODULE hSoundSys = GetModuleHandleW(L"soundsystem.dll");
            if (hSoundSys)
            {
                uintptr_t sosAddr = Mem::FindPattern(L"soundsystem.dll", Signatures::StartSoundEvent);
                if (sosAddr)
                {
                    pStartSoundEventTarget = reinterpret_cast<void*>(sosAddr);
                    MH_STATUS sst = MH_CreateHook(
                        pStartSoundEventTarget,
                        reinterpret_cast<void*>(&hkStartSoundEvent),
                        reinterpret_cast<void**>(&oStartSoundEvent));
                    if (sst == MH_OK || sst == MH_ERROR_ALREADY_CREATED)
                    {
                        MH_STATUS sen = MH_EnableHook(pStartSoundEventTarget);
                        sosHooked = (sen == MH_OK || sen == MH_ERROR_ENABLED);
                    }
                }
            }

            return hooked;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }

    // -----------------------------------------------------------
    // Shutdown â€” disable + remove hook, free WAV buffer.
    // -----------------------------------------------------------
    inline void Shutdown()
    {
        if (hooked && pKillFeedbackTarget)
        {
            MH_DisableHook(pKillFeedbackTarget);
            MH_RemoveHook(pKillFeedbackTarget);
        }
        hooked = false;
        pKillFeedbackTarget = nullptr;
        oKillFeedback = nullptr;

        if (dmgPatched && pDmgFeedbackTarget)
        {
            DWORD oldProt = 0;
            if (VirtualProtect(pDmgFeedbackTarget, 8, PAGE_EXECUTE_READWRITE, &oldProt))
            {
                auto* p = reinterpret_cast<uint8_t*>(pDmgFeedbackTarget);
                p[0] = g_dmgPatchOriginal[0];
                p[1] = g_dmgPatchOriginal[1];
                p[2] = g_dmgPatchOriginal[2];
                DWORD tmp; VirtualProtect(pDmgFeedbackTarget, 8, oldProt, &tmp);
                FlushInstructionCache(GetCurrentProcess(), pDmgFeedbackTarget, 8);
            }
        }
        dmgPatched = false;

        if (dmgHooked && pDmgFeedbackTarget)
        {
            MH_DisableHook(pDmgFeedbackTarget);
            MH_RemoveHook(pDmgFeedbackTarget);
        }
        dmgHooked = false;
        pDmgFeedbackTarget = nullptr;
        oDmgFeedback = nullptr;
        pGetHitGroup  = nullptr;

        if (emitHooked && pEmitSoundTarget)
        {
            MH_DisableHook(pEmitSoundTarget);
            MH_RemoveHook(pEmitSoundTarget);
        }
        emitHooked = false;
        pEmitSoundTarget = nullptr;
        oEmitSound = nullptr;

        if (sosHooked && pStartSoundEventTarget)
        {
            MH_DisableHook(pStartSoundEventTarget);
            MH_RemoveHook(pStartSoundEventTarget);
        }
        sosHooked = false;
        pStartSoundEventTarget = nullptr;
        oStartSoundEvent = nullptr;

        // Stop any in-flight playback before freeing the buffer
        PlaySoundA(nullptr, nullptr, 0);

        if (g_wav)
        {
            HeapFree(GetProcessHeap(), 0, g_wav);
            g_wav = nullptr;
            g_wavSize = 0;
        }
    }
}
