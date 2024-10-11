#include "utils.h"
#include "logger.h"
#include "stdafx.h"
#include "storage_hooks.h"

DWORD WINAPI InitThread(LPVOID lpParam) {
    Logger& l = Logger::Instance();
    uintptr_t base = reinterpret_cast<uintptr_t>(lpParam);
    InjectCode(base);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID /*lpReserved*/)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        Logger &l = Logger::Instance();
        l.Get()->info("DLL_PROCESS_ATTACH");
        l.Get()->flush();
        uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleA(NULL));
        IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
        if ((base + nt->OptionalHeader.AddressOfEntryPoint + (0x400000 - base)) == 0x4B88FD)
        {
            l.Get()->info("Correct EXE version detected. Creating initialization thread.");
            HANDLE hThread = CreateThread(NULL, 0, InitThread, reinterpret_cast<LPVOID>(base), 0, NULL);
            if (hThread)
            {
                CloseHandle(hThread);
            }
            else
            {
                l.Get()->error("Failed to create initialization thread.");
            }
            l.Get()->flush();
        }
        else
        {
            MessageBoxA(NULL, "This .exe is not supported.\nPlease use the Steam version", "Castle Crashers Custom Localizations", MB_ICONERROR);
            return FALSE;
        }
    }
    return TRUE;
}