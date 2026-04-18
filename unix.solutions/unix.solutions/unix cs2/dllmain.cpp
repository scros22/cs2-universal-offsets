#include <Windows.h>
#include <Psapi.h>
#include <thread>
#include <chrono>
#include <cstdio>

#include "hooks/present.h"
#include "hooks/menu.h"
#include "features/skin_changer.h"
#include "features/aimbot.h"
#include "features/esp.h"
#include "features/triggerbot.h"
#include "features/fov_changer.h"
#include "features/combat.h"
#include "features/chams.h"
#include "sdk/game.h"

#include <MinHook.h>

#pragma comment(lib, "Psapi.lib")

HMODULE g_hModule = nullptr;

void MainThread(HMODULE hModule)
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    printf("[+] UNIX CS2 Internal Loaded\n");

    Game::clientBase = Game::GetModuleBase(L"client.dll");
    if (!Game::clientBase)
    {
        printf("[-] Failed to get client.dll base\n");
        return;
    }

    printf("[+] client.dll: 0x%IX\n", Game::clientBase);
    printf("[*] Waiting for game modules...\n");

    while (!GetModuleHandleW(L"client.dll") || !GetModuleHandleW(L"engine2.dll") || !GetModuleHandleA("scenesystem.dll"))
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (!Game::Init())
    {
        printf("[-] Game::Init() FAILED\n");
        return;
    }

    printf("[+] Game::Init() OK\n");

    if (MH_Initialize() != MH_OK)
    {
        printf("[-] MinHook init FAILED\n");
        return;
    }

    if (!Hooks::SetupDXHooks())
    {
        printf("[-] DX hooks FAILED\n");
        return;
    }

    printf("[+] DX hooks OK (Present + ResizeBuffers)\n");

    if (Aimbot::Init())
    {
        printf("[+] Aimbot OK (CreateMove hooked)\n");
    }
    else
    {
        printf("[-] Aimbot init FAILED\n");
    }

    printf("[*] INSERT=menu, unload via Settings tab\n");
    FOVChanger::Init();
    Combat::Init();
    Chams::Setup();

    SkinChanger::running.store(true);
    while (SkinChanger::running.load())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (g_RequestUnload)
            break;
    }

    SkinChanger::running.store(false);
    FOVChanger::Shutdown();
    Combat::Shutdown();
    Aimbot::Shutdown();
    Hooks::Shutdown();
    MH_Uninitialize();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    FreeConsole();

    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        g_hModule = hModule;
        CloseHandle(CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread), hModule, 0, nullptr));
        break;
    case DLL_PROCESS_DETACH:
        SkinChanger::running.store(false);
        break;
    }
    return TRUE;
}
