#include "AttachSkinGraphicHook.h"
#include <HookCrashers.h> 
#include <detours.h>
#include <windows.h>
#include "../Core/CustomSaveManager.h"
#include "InitCharDataTableHook.h"

// --- Costanti ---
constexpr int ORIGINAL_BASE_CHAR_COUNT = 32;
constexpr int WORKSHOP_CHAR_COUNT = 10;
constexpr uintptr_t UI_CHAR_ARRAY_OFFSET = 0x11A8;

namespace CustomSave {
    constexpr uintptr_t AttachSkinGraphic_OFFSET = 0x8C820;
    using OriginalAttachSkinGraphic_t = void(__thiscall*)(void* thisPtr, int* pMovieClip, int contextId);
    static OriginalAttachSkinGraphic_t g_originalFunction = nullptr;

    void __fastcall DetouredAttachSkinGraphic(void* thisPtr, void* /* edxUnused */, int* pMovieClip, int contextId)
    {
        //ResetInitCharDataTableState();
        if (g_originalFunction) {
			g_originalFunction(thisPtr, pMovieClip, contextId);
        }
    }

    bool SetupAttachSkinGraphicHook(uintptr_t moduleBase) {
        HookCrashers::LogInfo("Setting up AttachSkinGraphic hook...");
        uintptr_t targetAddress = moduleBase + AttachSkinGraphic_OFFSET;
        g_originalFunction = reinterpret_cast<OriginalAttachSkinGraphic_t>(targetAddress);

        if (!g_originalFunction) {
            HookCrashers::LogError("Target address for AttachSkinGraphic is invalid.");
            return false;
        }

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (DetourAttach(&(PVOID&)g_originalFunction, DetouredAttachSkinGraphic) != NO_ERROR) {
            HookCrashers::LogError("DetourAttach for AttachSkinGraphic failed");
            DetourTransactionAbort();
            return false;
        }

        if (DetourTransactionCommit() != NO_ERROR) {
            HookCrashers::LogError("DetourTransactionCommit for AttachSkinGraphic failed");
            return false;
        }

        HookCrashers::LogInfo("AttachSkinGraphic hook attached successfully!");
        return true;
    }
}