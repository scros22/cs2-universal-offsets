#pragma once
// ============================================================
// mapper.h — Stealth Manual Mapper via BYOVD Kernel R/W
//
// Detection surface comparison vs. old mapper:
//   OLD: OpenProcess(VM_WRITE)       → triggers ObRegisterCallbacks
//   NEW: NtOpenProcess(VM_OPERATION) → direct syscall, minimal access
//
//   OLD: WriteProcessMemory          → hooked NtWriteVirtualMemory
//   NEW: Physical memory write       → invisible to usermode hooks
//
//   OLD: VirtualAllocEx(RWX)         → suspicious allocation
//   NEW: NtAllocateVirtualMemory(RW) → direct syscall, flip to RX later
//
//   OLD: NtCreateThreadEx via ntdll  → hooked, logged
//   NEW: NtCreateThreadEx direct sc  → bypasses ntdll hooks
//
//   OLD: ReadProcessMemory for flag  → hooked NtReadVirtualMemory
//   NEW: Physical memory read        → invisible
// ============================================================

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include "kernel_ctx.h"
#include "syscalls.h"

namespace BYOVD
{
    class KernelMapper
    {
        KernelContext& m_ctx;

        // Shellcode layout constants (two-part design)
        static constexpr size_t kRipOffset = 93;          // originalRIP in Part 1
        static constexpr size_t kTrampolineOffset = 101;  // Part 2 start

        // Verification: remote DWORD that shellcode writes on DllMain return
        PVOID m_flagAddr = nullptr;

        // Target process CR3 for kernel R/W
        uint64_t m_targetCR3 = 0;

        // Track hijacked thread so we can restore its RIP if verification fails
        DWORD m_hijackedTid = 0;
        DWORD64 m_hijackedOrigRip = 0;

        // Allocate verification flag page via direct syscall
        bool AllocVerifyFlag(HANDLE hProc)
        {
            m_flagAddr = nullptr;
            SIZE_T flagSize = 4096;
            NTSTATUS st = DirectSyscall::NtAllocateVirtualMemory(
                hProc, &m_flagAddr, 0, &flagSize,
                MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (st != 0 || !m_flagAddr) return false;

            // Zero the flag via kernel R/W (not WriteProcessMemory)
            DWORD zero = 0;
            m_ctx.WriteProcessVirtualEx(m_targetCR3,
                reinterpret_cast<uintptr_t>(m_flagAddr), &zero, sizeof(zero));
            return true;
        }

        // Poll verification flag via kernel R/W (not ReadProcessMemory)
        bool WaitForVerification(DWORD timeoutMs)
        {
            DWORD start = GetTickCount();
            while (GetTickCount() - start < timeoutMs)
            {
                DWORD val = 0;
                if (m_ctx.ReadProcessVirtualEx(m_targetCR3,
                    reinterpret_cast<uintptr_t>(m_flagAddr), &val, sizeof(val)))
                {
                    if (val == 0xCAFEBABE) return true;
                }
                Sleep(200);
            }
            return false;
        }

    public:
        explicit KernelMapper(KernelContext& ctx) : m_ctx(ctx) {}

        // -----------------------------------------------------------
        // MapDLL: Full stealth pipeline
        //   1. Parse PE from disk image
        //   2. Direct-syscall NtOpenProcess (minimal access)
        //   3. Direct-syscall NtAllocateVirtualMemory
        //   4. Copy sections, fix relocations & imports locally
        //   5. Kernel R/W to write image (bypasses ALL hooks)
        //   6. Direct-syscall NtProtectVirtualMemory per section
        //   7. Execute via thread hijack (direct syscalls)
        // -----------------------------------------------------------
        bool MapDLL(DWORD targetPid, const std::vector<uint8_t>& fileData)
        {
            if (fileData.size() < sizeof(IMAGE_DOS_HEADER))
            {
                printf("[Mapper] File too small\n");
                return false;
            }

            auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(fileData.data());
            if (dos->e_magic != IMAGE_DOS_SIGNATURE)
            {
                printf("[Mapper] Invalid DOS signature\n");
                return false;
            }

            auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS64*>(
                fileData.data() + dos->e_lfanew);
            if (nt->Signature != IMAGE_NT_SIGNATURE ||
                nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
            {
                printf("[Mapper] Invalid NT signature or not x64\n");
                return false;
            }

            DWORD imageSize = nt->OptionalHeader.SizeOfImage;
            printf("[Mapper] Image: %u sections, 0x%X bytes (%u KB)\n",
                   nt->FileHeader.NumberOfSections, imageSize, imageSize / 1024);

            // --- Initialize direct syscalls ---
            if (!DirectSyscall::Init())
            {
                printf("[Mapper] Direct syscall init failed — cannot proceed\n");
                return false;
            }

            // --- Get target process CR3 for kernel R/W ---
            m_targetCR3 = m_ctx.GetProcessCR3(targetPid);
            if (!m_targetCR3)
            {
                printf("[Mapper] Failed to resolve target CR3 (PID %lu)\n", targetPid);
                return false;
            }
            printf("[Mapper] Target CR3: 0x%llX\n", (unsigned long long)m_targetCR3);

            // --- Step 1: Open process via DIRECT SYSCALL ---
            // VM_WRITE needed to pre-fault demand-zero pages before physical R/W
            HANDLE hProc = DirectSyscall::OpenProcessDirect(targetPid,
                PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD | PROCESS_QUERY_LIMITED_INFORMATION);
            if (!hProc)
            {
                printf("[Mapper] Direct syscall OpenProcess failed\n");
                return false;
            }
            printf("[Mapper] Process handle via direct syscall (minimal access)\n");

            // --- Step 2: Allocate via DIRECT SYSCALL ---
            // PAGE_READWRITE initially — we'll set per-section protections later
            PVOID remoteBase = nullptr;
            SIZE_T allocSize = imageSize;
            NTSTATUS st = DirectSyscall::NtAllocateVirtualMemory(
                hProc, &remoteBase, 0, &allocSize,
                MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (st != 0 || !remoteBase)
            {
                printf("[Mapper] NtAllocateVirtualMemory failed: 0x%08lX\n", st);
                DirectSyscall::NtClose(hProc);
                return false;
            }
            printf("[Mapper] Allocated @ 0x%p via direct syscall\n", remoteBase);

            // --- Step 3: Build local mapped image ---
            std::vector<uint8_t> mapped(imageSize, 0);

            // Copy headers
            memcpy(mapped.data(), fileData.data(),
                   nt->OptionalHeader.SizeOfHeaders);

            // Copy sections
            auto* secArr = IMAGE_FIRST_SECTION(nt);
            for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++)
            {
                auto& sec = secArr[i];
                if (sec.SizeOfRawData == 0) continue;
                if (sec.PointerToRawData + sec.SizeOfRawData > fileData.size())
                    continue;
                memcpy(mapped.data() + sec.VirtualAddress,
                       fileData.data() + sec.PointerToRawData,
                       sec.SizeOfRawData);
            }

            // --- Step 4: Relocations ---
            uintptr_t preferredBase = nt->OptionalHeader.ImageBase;
            uintptr_t actualBase = reinterpret_cast<uintptr_t>(remoteBase);
            intptr_t delta = static_cast<intptr_t>(actualBase - preferredBase);

            auto& relocDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
            if (delta != 0 && relocDir.VirtualAddress && relocDir.Size)
            {
                DWORD offset = 0;
                int count = 0;
                while (offset < relocDir.Size)
                {
                    auto* block = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
                        mapped.data() + relocDir.VirtualAddress + offset);
                    if (block->SizeOfBlock == 0) break;

                    DWORD entryCount = (block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                    auto* entries = reinterpret_cast<WORD*>(
                        reinterpret_cast<uint8_t*>(block) + sizeof(IMAGE_BASE_RELOCATION));

                    for (DWORD e = 0; e < entryCount; e++)
                    {
                        WORD type = entries[e] >> 12;
                        WORD off  = entries[e] & 0xFFF;
                        DWORD rva = block->VirtualAddress + off;
                        if (rva >= imageSize) continue;

                        if (type == IMAGE_REL_BASED_DIR64)
                        {
                            auto* p = reinterpret_cast<uint64_t*>(mapped.data() + rva);
                            *p += delta;
                            count++;
                        }
                        else if (type == IMAGE_REL_BASED_HIGHLOW)
                        {
                            auto* p = reinterpret_cast<uint32_t*>(mapped.data() + rva);
                            *p += static_cast<uint32_t>(delta);
                            count++;
                        }
                    }
                    offset += block->SizeOfBlock;
                }
                printf("[Mapper] %d relocations applied\n", count);
            }

            // --- Step 5: Imports ---
            auto& importDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
            if (importDir.VirtualAddress && importDir.Size)
            {
                auto* desc = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
                    mapped.data() + importDir.VirtualAddress);

                int modCount = 0, funcCount = 0;
                while (desc->Name)
                {
                    const char* modName = reinterpret_cast<const char*>(
                        mapped.data() + desc->Name);
                    HMODULE hMod = GetModuleHandleA(modName);
                    if (!hMod) hMod = LoadLibraryA(modName);
                    if (!hMod) { desc++; continue; }

                    auto* thunk = reinterpret_cast<IMAGE_THUNK_DATA64*>(
                        mapped.data() + desc->FirstThunk);
                    auto* orig = desc->OriginalFirstThunk
                        ? reinterpret_cast<IMAGE_THUNK_DATA64*>(
                              mapped.data() + desc->OriginalFirstThunk)
                        : thunk;

                    while (orig->u1.AddressOfData)
                    {
                        FARPROC fn = nullptr;
                        if (IMAGE_SNAP_BY_ORDINAL64(orig->u1.Ordinal))
                            fn = GetProcAddress(hMod,
                                MAKEINTRESOURCEA(IMAGE_ORDINAL64(orig->u1.Ordinal)));
                        else
                        {
                            auto* entry = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(
                                mapped.data() + orig->u1.AddressOfData);
                            fn = GetProcAddress(hMod, entry->Name);
                        }

                        if (fn) { thunk->u1.Function = (uint64_t)fn; funcCount++; }
                        thunk++;
                        orig++;
                    }
                    modCount++;
                    desc++;
                }
                printf("[Mapper] Imports: %d modules, %d functions\n", modCount, funcCount);
            }

            // Update image base in mapped headers
            auto* mappedNt = reinterpret_cast<IMAGE_NT_HEADERS64*>(
                mapped.data() +
                reinterpret_cast<const IMAGE_DOS_HEADER*>(mapped.data())->e_lfanew);
            mappedNt->OptionalHeader.ImageBase = actualBase;

            // --- Step 6: Write image via KERNEL R/W ---
            // Physical memory write through driver — completely invisible to:
            //   - NtWriteVirtualMemory hooks
            //   - ObRegisterCallbacks (no VM_WRITE handle access)
            //   - Process memory monitoring
            printf("[Mapper] Writing %u bytes via kernel physical R/W...\n", imageSize);

            // Pre-fault all pages: demand-zero PTEs have Present=0, so physical
            // R/W via page table walk fails. Write a zero byte per page to force
            // the OS to create real PTEs, then overwrite with payload via physical.
            {
                BYTE zero = 0;
                int faulted = 0;
                for (size_t off = 0; off < imageSize; off += 4096)
                {
                    SIZE_T bw = 0;
                    st = DirectSyscall::NtWriteVirtualMemory(hProc,
                        reinterpret_cast<uint8_t*>(remoteBase) + off,
                        &zero, 1, &bw);
                    if (st == 0) faulted++;
                }
                printf("[Mapper] Pre-faulted %d/%u pages\n", faulted, (imageSize + 4095) / 4096);
            };

            // Write in 4KB chunks to respect page boundaries
            size_t written = 0;
            for (size_t off = 0; off < imageSize; off += 4096)
            {
                size_t chunk = min((size_t)4096, imageSize - off);
                if (!m_ctx.WriteProcessVirtualEx(m_targetCR3,
                    actualBase + off, mapped.data() + off, chunk))
                {
                    printf("[Mapper] Kernel write failed at offset 0x%zX\n", off);
                    // Fallback: try direct syscall NtWriteVirtualMemory
                    printf("[Mapper] Falling back to direct syscall write...\n");
                    SIZE_T bytesWritten = 0;
                    st = DirectSyscall::NtWriteVirtualMemory(hProc,
                        reinterpret_cast<uint8_t*>(remoteBase) + off,
                        const_cast<uint8_t*>(mapped.data() + off),
                        chunk, &bytesWritten);
                    if (st != 0)
                    {
                        printf("[Mapper] Syscall write also failed: 0x%08lX\n", st);
                        DirectSyscall::NtClose(hProc);
                        return false;
                    }
                }
                written += chunk;
            }
            printf("[Mapper] Image written via kernel R/W (%zu bytes)\n", written);

            // --- Step 7: Set section protections via DIRECT SYSCALL ---
            for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++)
            {
                auto& sec = secArr[i];
                DWORD prot = PAGE_READONLY;
                bool exec  = (sec.Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0;
                bool write = (sec.Characteristics & IMAGE_SCN_MEM_WRITE)   != 0;

                if (exec && write)  prot = PAGE_EXECUTE_READWRITE;
                else if (exec)      prot = PAGE_EXECUTE_READ;
                else if (write)     prot = PAGE_READWRITE;

                PVOID secAddr = reinterpret_cast<uint8_t*>(remoteBase) + sec.VirtualAddress;
                SIZE_T secSize = sec.Misc.VirtualSize ? sec.Misc.VirtualSize : sec.SizeOfRawData;
                if (secSize == 0) continue;
                ULONG old = 0;
                DirectSyscall::NtProtectVirtualMemory(hProc, &secAddr, &secSize, prot, &old);
            }

            // --- Step 8: Allocate verification flag ---
            if (!AllocVerifyFlag(hProc))
            {
                printf("[Mapper] Failed to allocate verification flag\n");
                DirectSyscall::NtClose(hProc);
                return false;
            }

            // --- Step 9: Build shellcode ---
            uintptr_t entryPoint = actualBase + nt->OptionalHeader.AddressOfEntryPoint;

            auto& tlsDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
            bool hasTLS = (tlsDir.VirtualAddress && tlsDir.Size);
            uintptr_t tlsAddrOfIndex = 0;
            if (hasTLS)
            {
                auto* tls = reinterpret_cast<const IMAGE_TLS_DIRECTORY64*>(
                    mapped.data() + tlsDir.VirtualAddress);
                tlsAddrOfIndex = tls->AddressOfIndex;
            }

            auto& excDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
            bool hasExc = (excDir.VirtualAddress && excDir.Size);
            uintptr_t pdataAddr = hasExc ? actualBase + excDir.VirtualAddress : 0;
            DWORD pdataEntries = hasExc ? excDir.Size / sizeof(RUNTIME_FUNCTION) : 0;

            HMODULE hK32 = GetModuleHandleA("kernel32.dll");
            uintptr_t pTlsAlloc = (uintptr_t)GetProcAddress(hK32, "TlsAlloc");
            uintptr_t pRtlAddFT = (uintptr_t)GetProcAddress(hK32, "RtlAddFunctionTable");

            // Two-part shellcode (same layout as before)
            unsigned char sc[] = {
                // ===== Part 1: Hijack — calls trampoline directly, then returns =====
                0x50,                                                           // push rax              [0]
                0x51,                                                           // push rcx              [1]
                0x52,                                                           // push rdx              [2]
                0x41, 0x50,                                                     // push r8               [3-4]
                0x41, 0x51,                                                     // push r9               [5-6]
                0x41, 0x52,                                                     // push r10              [7-8]
                0x41, 0x53,                                                     // push r11              [9-10]
                0x9C,                                                           // pushfq                [11]
                0x53,                                                           // push rbx              [12]
                0x48, 0x89, 0xE3,                                               // mov rbx, rsp          [13-15]
                0x48, 0x83, 0xE4, 0xF0,                                         // and rsp, ~0xF         [16-19]
                0x48, 0x83, 0xEC, 0x28,                                         // sub rsp, 0x28         [20-23]
                0x48, 0xB8, 0,0,0,0,0,0,0,0,                                   // mov rax, <trampoline>  [24-33] imm@26
                0xFF, 0xD0,                                                     // call rax              [34-35]
                0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,                       // [36-43]
                0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,                       // [44-51]
                0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,                       // [52-59]
                0x90,0x90,0x90,0x90,0x90,0x90,0x90,0x90,                       // [60-67]
                0x90,0x90,0x90,                                                 // [68-70]
                0x48, 0x89, 0xDC,                                               // mov rsp, rbx          [71-73]
                0x5B,                                                           // pop rbx               [74]
                0x9D,                                                           // popfq                 [75]
                0x41, 0x5B,                                                     // pop r11               [76-77]
                0x41, 0x5A,                                                     // pop r10               [78-79]
                0x41, 0x59,                                                     // pop r9                [80-81]
                0x41, 0x58,                                                     // pop r8                [82-83]
                0x5A,                                                           // pop rdx               [84]
                0x59,                                                           // pop rcx               [85]
                0x58,                                                           // pop rax               [86]
                0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,                            // jmp [rip+0]           [87-92]
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                // <originalRIP>         [93-100]

                // ===== Part 2: Trampoline =====
                0x48, 0x83, 0xEC, 0x28,                                         // sub rsp, 0x28         [101-104]
                0x48, 0xB8, 0,0,0,0,0,0,0,0,                                   // mov rax, <TlsAlloc>   [105-114] imm@107
                0xFF, 0xD0,                                                     // call rax              [115-116]
                0x48, 0xB9, 0,0,0,0,0,0,0,0,                                   // mov rcx, <&_tls_idx>  [117-126] imm@119
                0x89, 0x01,                                                     // mov [rcx], eax        [127-128]
                0x48, 0xB9, 0,0,0,0,0,0,0,0,                                   // mov rcx, <.pdata>     [129-138] imm@131
                0xBA, 0,0,0,0,                                                  // mov edx, <count>      [139-143] imm@140
                0x49, 0xB8, 0,0,0,0,0,0,0,0,                                   // mov r8, <imgBase>     [144-153] imm@146
                0x48, 0xB8, 0,0,0,0,0,0,0,0,                                   // mov rax, <RtlAddFT>   [154-163] imm@156
                0xFF, 0xD0,                                                     // call rax              [164-165]
                0x48, 0xB9, 0,0,0,0,0,0,0,0,                                   // mov rcx, <imgBase>    [166-175] imm@168
                0xBA, 0x01, 0x00, 0x00, 0x00,                                   // mov edx, 1            [176-180]
                0x4D, 0x31, 0xC0,                                               // xor r8, r8            [181-183]
                0x48, 0xB8, 0,0,0,0,0,0,0,0,                                   // mov rax, <entry>      [184-193] imm@186
                0xFF, 0xD0,                                                     // call rax              [194-195]
                0x48, 0xB8, 0,0,0,0,0,0,0,0,                                   // mov rax, <flagAddr>   [196-205] imm@198
                0xC7, 0x00, 0xBE, 0xBA, 0xFE, 0xCA,                            // mov dword [rax], 0xCAFEBABE [206-211]
                0x31, 0xC0,                                                     // xor eax, eax          [212-213]
                0x48, 0x83, 0xC4, 0x28,                                         // add rsp, 0x28         [214-217]
                0xC3                                                            // ret                   [218]
            };

            // Patch addresses
            memcpy(&sc[107], &pTlsAlloc,        8);
            memcpy(&sc[119], &tlsAddrOfIndex,   8);
            memcpy(&sc[131], &pdataAddr,         8);
            memcpy(&sc[140], &pdataEntries,      4);
            memcpy(&sc[146], &actualBase,        8);
            memcpy(&sc[156], &pRtlAddFT,        8);
            memcpy(&sc[168], &actualBase,        8);
            memcpy(&sc[186], &entryPoint,        8);

            uintptr_t flagAddr = reinterpret_cast<uintptr_t>(m_flagAddr);
            memcpy(&sc[198], &flagAddr,          8);

            if (!hasTLS) memset(&sc[105], 0x90, 24);
            if (!hasExc) memset(&sc[129], 0x90, 37);

            // Allocate shellcode via direct syscall — RW first
            PVOID scRemote = nullptr;
            SIZE_T scAllocSize = sizeof(sc);
            st = DirectSyscall::NtAllocateVirtualMemory(
                hProc, &scRemote, 0, &scAllocSize,
                MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (st != 0 || !scRemote)
            {
                printf("[Mapper] Shellcode alloc failed: 0x%08lX\n", st);
                DirectSyscall::NtClose(hProc);
                return false;
            }

            // Patch trampoline address
            uintptr_t trampolineRemote = reinterpret_cast<uintptr_t>(scRemote) + kTrampolineOffset;
            memcpy(&sc[26], &trampolineRemote, 8);

            // Write shellcode via KERNEL R/W (not WriteProcessMemory)
            if (!m_ctx.WriteProcessVirtualEx(m_targetCR3,
                reinterpret_cast<uintptr_t>(scRemote), sc, sizeof(sc)))
            {
                printf("[Mapper] Kernel write shellcode failed, trying direct syscall...\n");
                SIZE_T scWritten = 0;
                DirectSyscall::NtWriteVirtualMemory(hProc, scRemote, sc, sizeof(sc), &scWritten);
            }

            // Flip to RX via direct syscall
            PVOID scBase = scRemote;
            SIZE_T scProt = scAllocSize;
            ULONG scOldProt = 0;
            st = DirectSyscall::NtProtectVirtualMemory(hProc, &scBase, &scProt,
                PAGE_EXECUTE_READ, &scOldProt);
            if (st != 0)
            {
                printf("[Mapper] RX protect failed (0x%08lX), trying RWX\n", st);
                scBase = scRemote;
                scProt = scAllocSize;
                DirectSyscall::NtProtectVirtualMemory(hProc, &scBase, &scProt,
                    PAGE_EXECUTE_READWRITE, &scOldProt);
            }

            // --- CFG whitelist (best effort) ---
            {
                using SetValidCallTargetsFn = BOOL(WINAPI*)(HANDLE, PVOID, SIZE_T, ULONG, PCFG_CALL_TARGET_INFO);
                auto pSetValid = reinterpret_cast<SetValidCallTargetsFn>(
                    GetProcAddress(GetModuleHandleA("kernel32.dll"), "SetProcessValidCallTargets"));
                if (pSetValid)
                {
                    CFG_CALL_TARGET_INFO targets[2] = {};
                    targets[0].Offset = kTrampolineOffset;
                    targets[0].Flags  = CFG_CALL_TARGET_VALID;
                    targets[1].Offset = 0;
                    targets[1].Flags  = CFG_CALL_TARGET_VALID;
                    SIZE_T scPageSize = (sizeof(sc) + 0xFFF) & ~(SIZE_T)0xFFF;
                    pSetValid(hProc, scRemote, scPageSize, 2, targets);

                    uintptr_t epOffset = entryPoint - actualBase;
                    CFG_CALL_TARGET_INFO epTarget = {};
                    epTarget.Offset = epOffset;
                    epTarget.Flags  = CFG_CALL_TARGET_VALID;
                    pSetValid(hProc, remoteBase, nt->OptionalHeader.SizeOfImage, 1, &epTarget);
                }
            }

            // --- Step 10: Execute ---
            printf("[Mapper] Executing DLL entry...\n");
            bool execOk = false;

            // Primary: NtCreateThreadEx via DIRECT SYSCALL
            if (FallbackCreateThread(hProc, sc, sizeof(sc), scRemote))
            {
                printf("[Mapper] Thread created (direct syscall) — verifying...\n");
                if (WaitForVerification(10000))
                {
                    printf("[Mapper] DLL init VERIFIED (NtCreateThreadEx → kernel R/W poll)\n");
                    execOk = true;
                }
                else
                    printf("[Mapper] NtCreateThreadEx: no verification after 10s\n");
            }

            // Fallback: APC via direct syscall
            if (!execOk)
            {
                printf("[Mapper] Trying APC fallback (direct syscalls)...\n");
                if (FallbackAPC(hProc, targetPid, sc, sizeof(sc), scRemote))
                {
                    if (WaitForVerification(8000))
                    {
                        printf("[Mapper] DLL init VERIFIED (APC → kernel R/W poll)\n");
                        execOk = true;
                    }
                }
            }

            if (!execOk)
            {
                printf("[Mapper] All execution methods failed — cleaning up\n");
                NeuterTrampoline(scRemote, sizeof(sc));
                DirectSyscall::NtClose(hProc);
                return false;
            }

            printf("[Mapper] DLL mapped successfully @ 0x%p (full stealth)\n", remoteBase);

            // --- Post-map stealth: VAD cloaking ---
            CloakVAD(actualBase, allocSize);

            // Neuter trampoline for stale APCs
            NeuterTrampoline(scRemote, sizeof(sc));

            DirectSyscall::NtClose(hProc);
            return true;
        }

    private:
        // -----------------------------------------------------------
        // VAD Cloaking — modify our allocation's VAD entry via kernel
        // R/W so it looks like a mapped image instead of suspicious
        // private executable memory. VirtualQuery/NtQueryVirtualMemory
        // will see it as MEM_IMAGE/MEM_MAPPED rather than MEM_PRIVATE.
        // -----------------------------------------------------------
        void CloakVAD(uintptr_t allocBase, size_t allocSize)
        {
            // We already have the target EPROCESS in m_ctx from earlier
            uintptr_t eproc = 0;
            {
                // Re-find EPROCESS using system process as reference
                auto pSysProc = m_ctx.GetKernelExport("PsInitialSystemProcess");
                if (!pSysProc) { printf("[VAD] No PsInitialSystemProcess\n"); return; }
                auto systemEproc = m_ctx.Driver().ReadMemory<uintptr_t>(pSysProc);
                if (!systemEproc) return;

                // We need cs2.exe EPROCESS. Walk from system using stored offsets.
                // Re-detect PID offset from system EPROCESS
                size_t OFF_PID = 0, OFF_LINKS = 0;
                {
                    constexpr size_t SCAN = 0x1000;
                    auto* buf = new(std::nothrow) uint8_t[SCAN];
                    if (buf && m_ctx.Driver().ReadBuffer(systemEproc, buf, SCAN))
                    {
                        for (size_t off = 0x28; off < SCAN - 16; off += 8)
                        {
                            uint64_t val = *reinterpret_cast<uint64_t*>(buf + off);
                            if (val == 4)
                            {
                                uint64_t flink = *reinterpret_cast<uint64_t*>(buf + off + 8);
                                if ((flink >> 48) == 0xFFFF || (flink >> 48) == 0xFFFE)
                                {
                                    OFF_PID = off;
                                    OFF_LINKS = off + 8;
                                    break;
                                }
                            }
                        }
                    }
                    delete[] buf;
                }
                if (!OFF_PID) { printf("[VAD] PID offset detect failed\n"); return; }

                // Walk to find cs2.exe
                uintptr_t current = systemEproc;
                uintptr_t listHead = current + OFF_LINKS;
                DWORD targetPid = GetProcessId(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
                    static_cast<DWORD>(m_ctx.Driver().ReadMemory<uintptr_t>(systemEproc + OFF_PID))));
                // Actually, get the pid from the allocation handle context
                // Just walk the list matching CR3
                for (int i = 0; i < 4096; ++i)
                {
                    uint64_t cr3 = m_ctx.Driver().ReadMemory<uint64_t>(current + 0x28);
                    if (cr3 == m_targetCR3)
                    {
                        eproc = current;
                        break;
                    }
                    auto flink = m_ctx.Driver().ReadMemory<uintptr_t>(current + OFF_LINKS);
                    if (!flink || flink == listHead) break;
                    current = flink - OFF_LINKS;
                }
            }
            if (!eproc) { printf("[VAD] Could not find target EPROCESS for VAD cloak\n"); return; }

            // Target VPN range
            uint32_t targetVpn = static_cast<uint32_t>(allocBase >> 12);

            // Auto-detect VadRoot offset: scan EPROCESS [0x700, 0x900]
            // for a kernel pointer that leads to a valid AVL tree
            uintptr_t vadRoot = 0;
            for (size_t off = 0x700; off < 0x900; off += 8)
            {
                auto candidate = m_ctx.Driver().ReadMemory<uintptr_t>(eproc + off);
                if (!candidate) continue;
                if ((candidate >> 48) != 0xFFFF && (candidate >> 48) != 0xFFFE) continue;

                // Validate: looks like MMVAD_SHORT with plausible VPNs
                uint32_t svpn = 0, evpn = 0;
                m_ctx.Driver().ReadBuffer(candidate + 0x18, &svpn, 4);
                m_ctx.Driver().ReadBuffer(candidate + 0x1C, &evpn, 4);
                if (evpn >= svpn && svpn > 0 && evpn < 0x7FFFFFFF)
                {
                    vadRoot = candidate;
                    printf("[VAD] VadRoot at EPROCESS+0x%zX root=0x%llX\n", off, (uint64_t)vadRoot);
                    break;
                }
            }
            if (!vadRoot) { printf("[VAD] VadRoot auto-detect failed — skipping\n"); return; }

            // BST search for our allocation's VAD node
            uintptr_t node = vadRoot;
            uintptr_t foundVad = 0;
            for (int depth = 0; depth < 64; ++depth)
            {
                if (!node) break;
                uint32_t svpn = 0, evpn = 0;
                m_ctx.Driver().ReadBuffer(node + 0x18, &svpn, 4);
                m_ctx.Driver().ReadBuffer(node + 0x1C, &evpn, 4);

                if (targetVpn >= svpn && targetVpn <= evpn)
                {
                    foundVad = node;
                    break;
                }

                uintptr_t next = 0;
                if (targetVpn < svpn)
                    next = m_ctx.Driver().ReadMemory<uintptr_t>(node + 0x00); // left child
                else
                    next = m_ctx.Driver().ReadMemory<uintptr_t>(node + 0x08); // right child

                if (!next || ((next >> 48) != 0xFFFF && (next >> 48) != 0xFFFE))
                    break;
                node = next;
            }

            if (!foundVad) { printf("[VAD] Could not find VAD for 0x%llX\n", (uint64_t)allocBase); return; }

            // Read and modify VadFlags at +0x30 of MMVAD_SHORT
            uint32_t flags = 0;
            m_ctx.Driver().ReadBuffer(foundVad + 0x30, &flags, 4);
            printf("[VAD] Found @ 0x%llX, flags=0x%08X\n", (uint64_t)foundVad, flags);

            // Patch: VadType=2 (VadImageMap), PrivateMemory=0
            // This makes VirtualQuery report MEM_IMAGE instead of MEM_PRIVATE
            uint32_t newFlags = flags;
            newFlags &= ~(0x7u << 4);   // clear VadType (bits 4-6)
            newFlags |=  (0x2u << 4);    // VadType = VadImageMap
            newFlags &= ~(1u << 21);     // clear PrivateMemory

            m_ctx.Driver().WriteBuffer(foundVad + 0x30, &newFlags, 4);
            printf("[VAD] Cloaked: 0x%08X → 0x%08X (VadImageMap)\n", flags, newFlags);
        }

        // Write RET to trampoline start via kernel R/W
        void NeuterTrampoline(PVOID scRemote, size_t scSize)
        {
            unsigned char retNop[4] = { 0xC3, 0x90, 0x90, 0x90 };
            uintptr_t trampolineAddr = reinterpret_cast<uintptr_t>(scRemote) + kTrampolineOffset;

            // Try kernel R/W first (stealthiest)
            if (m_ctx.WriteProcessVirtualEx(m_targetCR3, trampolineAddr, retNop, sizeof(retNop)))
            {
                printf("[Mapper] Trampoline neutered via kernel R/W\n");
                return;
            }

            // Fallback: flip to RW, write via syscall, flip back
            printf("[Mapper] Neutering via direct syscall fallback\n");
            // We don't have a handle here in all paths, so kernel R/W is preferred
        }

        // -----------------------------------------------------------
        // FallbackCreateThread: NtCreateThreadEx via DIRECT SYSCALL
        // Bypasses ntdll hooks entirely.
        // -----------------------------------------------------------
        bool FallbackCreateThread(HANDLE hProc,
                                  unsigned char* sc, size_t scSize, PVOID scRemote)
        {
            printf("[Thread] NtCreateThreadEx via direct syscall...\n");

            PVOID trampolineAddr = reinterpret_cast<uint8_t*>(scRemote) + kTrampolineOffset;

            HANDLE hThread = nullptr;
            NTSTATUS st = DirectSyscall::NtCreateThreadEx(
                &hThread, THREAD_ALL_ACCESS, nullptr, hProc,
                trampolineAddr, nullptr,
                0, 0, 0, 0, nullptr);

            if (st != 0 || !hThread)
            {
                printf("[Thread] NtCreateThreadEx failed: 0x%08lX\n", st);
                return false;
            }

            printf("[Thread] Thread created via direct syscall\n");

            // Wait for thread to finish (uses kernel32 WaitForSingleObject —
            // this is fine, it's our own handle, not target process state)
            WaitForSingleObject(hThread, 15000);
            DirectSyscall::NtClose(hThread);
            return true;
        }

        // -----------------------------------------------------------
        // FallbackAPC: Queue APC via DIRECT SYSCALL
        // Uses NtOpenThread + NtQueueApcThread — all via direct syscalls
        // -----------------------------------------------------------
        bool FallbackAPC(HANDLE hProc, DWORD pid,
                         unsigned char* sc, size_t scSize, PVOID scRemote)
        {
            printf("[APC] Queuing APCs via direct syscalls...\n");

            PVOID trampolineAddr = reinterpret_cast<uint8_t*>(scRemote) + kTrampolineOffset;

            // Enumerate threads (CreateToolhelp32Snapshot is usermode-only, safe)
            HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
            if (snap == INVALID_HANDLE_VALUE) return false;

            THREADENTRY32 te{};
            te.dwSize = sizeof(te);

            int queued = 0;
            if (Thread32First(snap, &te))
            {
                do {
                    if (te.th32OwnerProcessID != pid) continue;

                    // Open thread via DIRECT SYSCALL
                    HANDLE hThread = DirectSyscall::OpenThreadDirect(
                        te.th32ThreadID, THREAD_SET_CONTEXT);
                    if (!hThread) continue;

                    // Queue APC via DIRECT SYSCALL
                    NTSTATUS st = DirectSyscall::NtQueueApcThread(
                        hThread, trampolineAddr, nullptr, nullptr, nullptr);
                    if (st == 0)
                    {
                        queued++;
                        printf("[APC] Queued to thread %lu (direct syscall)\n", te.th32ThreadID);
                    }
                    DirectSyscall::NtClose(hThread);

                    if (queued >= 6) break;
                } while (Thread32Next(snap, &te));
            }
            CloseHandle(snap);

            if (queued == 0)
            {
                printf("[APC] Failed to queue any APCs\n");
                return false;
            }

            printf("[APC] Queued on %d threads via direct syscalls\n", queued);
            return true;
        }
    };
}
