// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include "mem.h"
#include "proc.h"

// t in twglSwapBuffers stands for template
// this is a pointer to the function
typedef BOOL(__stdcall* twglSwapBuffers) (HDC hDc); 

// pointer to the original function, hence the "o"
twglSwapBuffers owglSwapBuffers;

// hook function, hence the "hk"
BOOL __stdcall hkwglSwapBufers(HDC hDc) {
    std::cout << "Hooked" << std::endl;
    return owglSwapBuffers(hDc); // call to original function
}

DWORD WINAPI HackThread(HMODULE hModule) {

    // Create console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    std::cout << "OG for a fee, stay sippin' fam\n";

    // get module base
    uintptr_t moduleBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");
    bool bHealth = false, bAmmo = false, bRecoil = false;

    //

    owglSwapBuffers = (twglSwapBuffers)GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers");
    owglSwapBuffers = (twglSwapBuffers)mem::TrampHook32((BYTE*)owglSwapBuffers, (BYTE*)hkwglSwapBufers, 5);

    //

    // hack loop
    while (true) { // key input
        if (GetAsyncKeyState(VK_F4) & 1) {
            break;
        }
        if (GetAsyncKeyState(VK_F1) & 1) {
            bHealth = !bHealth;
        }
        if (GetAsyncKeyState(VK_F2) & 1) {
            bAmmo = !bAmmo;
        }
        if (GetAsyncKeyState(VK_F3) & 1) {
            bRecoil = !bRecoil;

            if (bRecoil) {
                // nop
                mem::Nop((BYTE*)(moduleBase + 0x63786), 10);
            }
            else {
                // write back original instructions
                mem::Patch((BYTE*)(moduleBase + 0x63786), (BYTE*)"\x50\x8d\x4c\x24\x1c\x51\x8b\xce\xff\xd2", 10);

            }
        }
        // cont write/freeze
        uintptr_t* localPlayerPtr = (uintptr_t*)(moduleBase + 0x10f4f4);
        if (localPlayerPtr) {
            if (bHealth) {
                *(int*)(*localPlayerPtr + 0xf8) = 1337;
            }
            if (bAmmo) {
                uintptr_t ammoAddr = mem::FindDMAAddy(moduleBase + 0x10f4f4, { 0x384, 0x14, 0x0 });
                int* ammo = (int*)ammoAddr;
                *ammo = 1337;
            }
        }
        Sleep(5);
    }

    // cleanup and eject
    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;

}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr));
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

