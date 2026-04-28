#pragma once

// ---------------------------------------------------------------
// Kill Sound — replaces Valve's headshot/kill ding with a custom
// short metallic "ding".
//
// Hooks the per-kill AttackerFeedback emitter in client.dll
// (sub_180849CE0 on build 14155). That single function is the
// dispatch point for all four "Player.Death*.AttackerFeedback"
// sound events — head/body × armor/no-armor. By short-circuiting
// it (return 0, skip the engine sound emit) and triggering our
// own PlaySoundA from a synthesized in-memory WAV, we get:
//
//   * Valve's headshot kill ding muted (the iconic CS2 ding that
//     plays on every HS kill — server-side telemetry doesn't see
//     this, so it's purely a client-side cosmetic change).
//   * Custom satisfying ding on EVERY confirmed kill (head OR
//     body), driven by the engine's own kill detection flow —
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
#include "../core/memory.h"
#include "../core/signatures.h"
#include "../vendor/minhook/include/MinHook.h"

namespace KillSound
{
    struct Config
    {
        bool  enabled = true;       // master toggle
        bool  muteValve = true;     // skip Valve's HS/body kill ding
        float volume = 0.55f;       // 0..1 — playback amplitude scalar baked into WAV
    };
    inline Config cfg;

    // Hook plumbing
    using KillFeedbackFn = __int64(__fastcall*)(__int64, __int64);
    inline KillFeedbackFn oKillFeedback = nullptr;
    inline void* pKillFeedbackTarget = nullptr;
    inline bool  hooked = false;

    // Synthesized WAV buffer (heap-allocated, owned for cheat lifetime)
    inline uint8_t* g_wav = nullptr;
    inline int      g_wavSize = 0;

    // Stat counter (diagnostics — number of kill events seen)
    inline std::atomic<uint32_t> g_kills{ 0 };

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

        // Decay constants — sharp attack (~3ms), exponential tail
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
    // PlayDing — fire-and-forget. winmm queues to mixer, returns
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
    // Detour — runs on every confirmed kill (the engine calls this
    // from CCSPlayer_Pawn damage/death dispatch). We:
    //   1. bump the kill counter
    //   2. play our ding (cfg.enabled gate)
    //   3. either skip Valve's emit (cfg.muteValve) or pass through
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

        if (cfg.enabled && cfg.muteValve)
            return 0; // skip Valve's sound event emit entirely

        // Pass through — Valve's ding still plays on top of ours
        if (oKillFeedback)
            return oKillFeedback(a1, a2);
        return 0;
    }

    // -----------------------------------------------------------
    // Setup — sigscan + MinHook install. Idempotent.
    // -----------------------------------------------------------
    inline bool Setup()
    {
        if (hooked) return true;

        if (!BuildDingWav())
            return false;

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
            return hooked;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }

    // -----------------------------------------------------------
    // Shutdown — disable + remove hook, free WAV buffer.
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
