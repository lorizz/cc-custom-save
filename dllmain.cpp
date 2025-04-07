#include "utils.h"
#include "logger.h"
#include "stdafx.h"
#include "storage_hooks.h"
#include "dynamic_character_data.h"
#include "default_character_data.h"
#include "addon_characters.h"

std::vector<FieldInfo> gCharacterStructure;
DWORD WINAPI InitThread(LPVOID lpParam) {
    Logger& l = Logger::Instance();
    uintptr_t base = reinterpret_cast<uintptr_t>(lpParam);

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string basePath = std::string(exePath);
    basePath = basePath.substr(0, basePath.find_last_of("\\/"));

    std::string iniPath = basePath + "\\scripts\\CustomSave.ini";

    l.Get()->info("Executable path: {}", basePath);
    l.Get()->info("INI path: {}", iniPath);
    l.Get()->flush();

    l.Get()->info("Loading INI file...");
    l.Get()->flush();
    gCharacterStructure = readCharacterDataStructure(iniPath.c_str());
    if (gCharacterStructure.empty()) {
        l.Get()->error("Failed to load character data structure from INI file.");
        l.Get()->flush();
        return 1;
    }
    l.Get()->info("Character data structure loaded successfully. Total fields: {}", gCharacterStructure.size());
    l.Get()->flush();

    defaultCharacters = InitializeDefaultCharacters();
    l.Get()->info("Default characters initialized: {}", defaultCharacters.size());
    l.Get()->flush();

    InitializeAddonCharacters(iniPath.c_str());
    l.Get()->info("Addon characters loaded, total addon characters: {}", addonCharacters.size());
    l.Get()->flush();

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

        char currentDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, currentDir);
        l.Get()->info("Current working directory: {}", currentDir);
        l.Get()->flush();
        uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandleA(NULL));
        IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
        if ((base + nt->OptionalHeader.AddressOfEntryPoint + (0x400000 - base)) == 0x4B88FD ||
            (base + nt->OptionalHeader.AddressOfEntryPoint + (0x400000 - base)) == 0x730310) // Modded is 0x4B88FD, Original is 0x730310
        {
            //MessageBoxA(NULL, "DEBUG PAUSE", "Castle Crashers Custom Save", MB_ICONINFORMATION);
            l.Get()->info("Correct EXE version detected. Creating initialization thread. Base address 0x{:02}", base);
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
            MessageBoxA(NULL, "This .exe is not supported.\nPlease use the Steam version", "Castle Crashers Custom Save", MB_ICONERROR);
            return FALSE;
        }
    }
    return TRUE;
}