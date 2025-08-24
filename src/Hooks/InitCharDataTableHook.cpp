#include "InitCharDataTableHook.h"
#include <HookCrashersAPI.h> 
#include <detours.h>
#include <windows.h>
#include "../Core/CustomSaveManager.h"

namespace CustomSave {

    constexpr uintptr_t InitCharDataTable_OFFSET = 0x83C10;

    using OriginalInitCharDataTable_t = void(__thiscall*)(void* thisPtr, char param1);
    static OriginalInitCharDataTable_t g_originalFunction = nullptr;

    // --- Il nostro Hook ---
    void __fastcall DetouredInitCharDataTable(void* thisPtr, void* /* edxUnused */, char param1)
    {
        CustomSave::ResetGraphicShiftState();
        if (g_originalFunction != nullptr) {
            g_originalFunction(thisPtr, param1);
        }
    }

    bool SetupInitCharDataTableHook(uintptr_t moduleBase) {
        HookCrashers::API::Client::LogInfo("Setting up InitCharDataTable hook...");

        uintptr_t targetAddress = moduleBase + InitCharDataTable_OFFSET;
        g_originalFunction = reinterpret_cast<OriginalInitCharDataTable_t>(targetAddress);

        if (!g_originalFunction) {
            HookCrashers::API::Client::LogError("Target address for InitCharDataTable is invalid.");
            return false;
        }

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        if (DetourAttach(&(PVOID&)g_originalFunction, DetouredInitCharDataTable) != NO_ERROR) {
            HookCrashers::API::Client::LogError("DetourAttach for InitCharDataTable failed");
            DetourTransactionAbort();
            return false;
        }
        if (DetourTransactionCommit() != NO_ERROR) {
            HookCrashers::API::Client::LogError("DetourTransactionCommit for InitCharDataTable failed");
            return false;
        }

        HookCrashers::API::Client::LogInfo("InitCharDataTable hook attached successfully!");
        return true;
    }
}