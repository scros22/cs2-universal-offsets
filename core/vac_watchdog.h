#pragma once

// =============================================================================
// vac_watchdog.h — Untrusted-mode panic detector
// -----------------------------------------------------------------------------
// CS2 dispatches "untrusted" / VAC-related disconnect events through
// ClientModeCSNormal::OnEvent (build 14154 RVA 0xC5A0B0, addr 0x180C5A0B0).
// IDA-verified — see decompile of sub_180C5A0B0.  The function inspects
// KeyValues::GetName(a2) and branches on these names:
//
//      "OnClientInsecureBlocked"          — VAC server kicked us
//      "OnClientUntrustedLaunch"          — unsigned/injected module detected
//      "OnClientUntrustedSystemFiles"     — tampered game files / cheat module
//      "OnClientUntrustedDisallowed"      — actively disallowed launch
//      "OnTrustedLaunchFailed"            — trusted-mode init failed
//      "OnClientPureFileStateDirty"       — pure-server file mismatch
//
// When ANY of these fire the client is already flagged.  We hook the
// dispatcher so we can:
//   1. Log + telemetry the trigger
//   2. Run Stealth::cleanupFn (unhook everything) BEFORE the dialog renders
//   3. Optionally swallow the event (just call original with a renamed KV)
//
// We do NOT swallow by default — that would cause additional integrity checks
// to fire and is more suspicious than just disappearing.  Cleaning up + waiting
// for the user to alt-f4 is the cleanest path.
//
// Sig (uniquely matches sub_180C5A0B0 on build 14154):
//   40 53 57 48 81 EC 78 02 00 00 48 8B CA 48 8B FA
// =============================================================================

#include <Windows.h>
#include <cstdint>
#include <cstring>
#include <atomic>

#include "../vendor/minhook/include/MinHook.h"
#include "memory.h"
#include "stealth.h"
#include "xorstr.h"

namespace VacWatchdog
{
    // ---- KeyValues::GetName resolution (tier0.dll export) -------------------
    using GetName_t = const char* (__fastcall*)(void* /*KeyValues* */);
    inline GetName_t pGetName = nullptr;

    // ---- Original / hook plumbing ------------------------------------------
    using OnEvent_t = void (__fastcall*)(void* /*ClientModeCSNormal* */, void* /*KeyValues* */);
    inline OnEvent_t  oOnEvent       = nullptr;
    inline void*      pOnEventTarget = nullptr;

    inline std::atomic<bool> tripped{false};
    inline std::atomic<int>  tripReason{0};
    inline std::atomic<uint64_t> lastTripTick{0};

    // ---- Logging ------------------------------------------------------------
    // DEBUG-only. Named %TEMP% files written by an injected process on a
    // heartbeat are an untrusted-mode (20-hour cooldown) signal; release
    // builds never touch disk here. Body kept so call sites need no #ifdef.
    inline void Log(const char* fmt, ...)
    {
#ifdef _DEBUG
        char path[MAX_PATH];
        GetTempPathA(MAX_PATH, path);
        lstrcatA(path, "cs2vac.txt");
        HANDLE hFile = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ,
                                   nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return;
        char buf[512];
        va_list args; va_start(args, fmt);
        int len = vsprintf_s(buf, sizeof(buf) - 2, fmt, args);
        va_end(args);
        if (len > 0) { buf[len] = '\n'; buf[len + 1] = '\0'; DWORD w; WriteFile(hFile, buf, len + 1, &w, nullptr); }
        CloseHandle(hFile);
#else
        (void)fmt;
#endif
    }

    // ---- Trip table — events that mean "we are detected" --------------------
    // We intentionally classify pure-file-dirty as a soft trip (not from VAC,
    // but from sv_pure mismatch — eject anyway because hooks may be the cause).
    enum class Trip : int
    {
        None                      = 0,
        InsecureBlocked           = 1,
        UntrustedLaunch           = 2,
        UntrustedSystemFiles      = 3,
        UntrustedDisallowed       = 4,
        TrustedLaunchFailed       = 5,
        PureFileStateDirty        = 6,
    };

    inline Trip Classify(const char* name)
    {
        if (!name) return Trip::None;
        // Compare against XOR-encoded copies so the literal
        // "OnClientInsecureBlocked" / "OnClientUntrustedLaunch" /
        // "OnClientUntrustedSystemFiles" / "OnClientUntrustedDisallowed" /
        // "OnTrustedLaunchFailed" / "OnClientPureFileStateDirty"
        // strings never live in our .rdata. Those exact strings are a
        // YARA / public-cheat fingerprint; carrying them plaintext is a
        // free signal toward the untrusted-mode (20-hour) cooldown.
        // Decoded once per call into stack-local buffers.
        if (lstrcmpA(name, XS("OnClientInsecureBlocked"))      == 0) return Trip::InsecureBlocked;
        if (lstrcmpA(name, XS("OnClientUntrustedLaunch"))      == 0) return Trip::UntrustedLaunch;
        if (lstrcmpA(name, XS("OnClientUntrustedSystemFiles")) == 0) return Trip::UntrustedSystemFiles;
        if (lstrcmpA(name, XS("OnClientUntrustedDisallowed"))  == 0) return Trip::UntrustedDisallowed;
        if (lstrcmpA(name, XS("OnTrustedLaunchFailed"))        == 0) return Trip::TrustedLaunchFailed;
        if (lstrcmpA(name, XS("OnClientPureFileStateDirty"))   == 0) return Trip::PureFileStateDirty;
        return Trip::None;
    }

    // ---- Hook ---------------------------------------------------------------
    inline void __fastcall hkOnEvent(void* self, void* kv)
    {
        // Best-effort name read — pGetName resolved at install time.  If
        // resolution failed we still call original to avoid crashing CS2.
        if (pGetName && kv)
        {
            __try
            {
                const char* name = pGetName(kv);
                Trip t = Classify(name);
                if (t != Trip::None)
                {
                    tripped.store(true);
                    tripReason.store(static_cast<int>(t));
                    lastTripTick.store(GetTickCount64());
                    Log("[VAC] PANIC trip=%d name=%s \u2014 running cleanup",
                        static_cast<int>(t), name ? name : "(null)");

                    // PureFileStateDirty is sv_pure mismatch \u2014 NOT a real
                    // detection of us. Running emergency cleanup for it is
                    // counter-productive: it tears down all hooks for what
                    // is effectively a server-config issue, and the abrupt
                    // unhook itself (followed by re-hook on map change) is
                    // a stronger fingerprint than the pure event ever was.
                    // Log it, mark it, but do NOT cleanup.
                    if (t == Trip::PureFileStateDirty)
                    {
                        // fall through to forward-original below
                    }
                    else if (Stealth::cleanupFn && !Stealth::cleanupDone)
                    {
                        Stealth::cleanupDone = true;
                        __try { Stealth::cleanupFn(); }
                        __except(EXCEPTION_EXECUTE_HANDLER) {}
                    }
                }
            }
            __except(EXCEPTION_EXECUTE_HANDLER) { /* swallow — never crash CS2 */ }
        }

        // Always forward to original — silently swallowing would itself be
        // suspicious (the GC expects an ack on some of these events).
        if (oOnEvent) oOnEvent(self, kv);
    }

    // ---- Install ------------------------------------------------------------
    inline std::atomic<bool> installed{false};

    inline bool Install()
    {
        if (installed.load()) return true;

        // Resolve KeyValues::GetName from tier0.dll.  This dll is normally
        // loaded by CS2 long before us; if not, retry-on-heartbeat.
        HMODULE hTier0 = GetModuleHandleW(L"tier0.dll");
        if (!hTier0)
        {
            Log("[VAC] tier0.dll not loaded yet — deferring");
            return false;
        }

        pGetName = reinterpret_cast<GetName_t>(
            GetProcAddress(hTier0, "?GetName@KeyValues@@QEBAPEBDXZ"));
        if (!pGetName)
        {
            Log("[VAC] tier0!KeyValues::GetName export missing — aborting");
            return false;
        }

        // Sigscan ClientModeCSNormal::OnEvent in client.dll.
        // Build 14154 prologue (verified in IDA at 0x180C5A0B0):
        //   40 53                    push rbx
        //   57                       push rdi
        //   48 81 EC 78 02 00 00     sub rsp, 278h
        //   48 8B CA                 mov rcx, rdx
        //   48 8B FA                 mov rdi, rdx
        // The full window is uniquely identifying (single match across .text).
        static const char* candidates[] = {
            "40 53 57 48 81 EC 78 02 00 00 48 8B CA 48 8B FA",
            // Legacy fallback (older builds had no `mov rdi,rdx` chain):
            "40 53 57 48 81 EC 78 02 00 00 48 8B CA",
        };

        uintptr_t addr = 0;
        for (const char* sig : candidates)
        {
            addr = Mem::FindPattern(L"client.dll", sig);
            if (addr) break;
        }
        if (!addr)
        {
            Log("[VAC] ClientModeCSNormal::OnEvent NOT FOUND — sig drift");
            return false;
        }

        MH_STATUS st = MH_CreateHook(reinterpret_cast<void*>(addr),
                                     reinterpret_cast<void*>(&hkOnEvent),
                                     reinterpret_cast<void**>(&oOnEvent));
        if (st != MH_OK) { Log("[VAC] MH_CreateHook failed: %d", st); return false; }

        st = MH_EnableHook(reinterpret_cast<void*>(addr));
        if (st != MH_OK) { Log("[VAC] MH_EnableHook failed: %d", st); return false; }

        pOnEventTarget = reinterpret_cast<void*>(addr);
        installed.store(true);
        Log("[VAC] Watchdog armed @ 0x%p (tier0!GetName=0x%p)",
            (void*)addr, (void*)pGetName);
        return true;
    }

    inline void Shutdown()
    {
        if (!installed.exchange(false)) return;
        if (pOnEventTarget)
        {
            MH_DisableHook(pOnEventTarget);
            MH_RemoveHook(pOnEventTarget);
            pOnEventTarget = nullptr;
        }
        oOnEvent = nullptr;
        pGetName = nullptr;
    }

    // ---- Heartbeat tick — try to install if deferred ------------------------
    inline void Tick()
    {
        if (!installed.load())
        {
            // Throttle to ~1 attempt every ~2s (called from 80ms loop).
            static int skip = 0;
            if (++skip < 25) return;
            skip = 0;
            Install();
        }
    }
}
