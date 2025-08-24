#include "AttachSkinGraphicHook.h" // Crea un nuovo file per questo hook
#include <HookCrashersAPI.h> 
#include <detours.h>
#include <windows.h>
#include "../Core/CustomSaveManager.h"

// --- Costanti ---
constexpr int ORIGINAL_BASE_CHAR_COUNT = 32;
constexpr uintptr_t UI_CHAR_ARRAY_OFFSET = 0x11A8;

namespace CustomSave {
    constexpr uintptr_t AttachSkinGraphic_OFFSET = 0x8C820;

    using OriginalAttachSkinGraphic_t = void(__thiscall*)(void* thisPtr, int* pMovieClip, int contextId);
    static OriginalAttachSkinGraphic_t g_originalFunction = nullptr;

    bool hasBeenShiftedThisFrame = false;

    void __fastcall DetouredAttachSkinGraphic(void* thisPtr, void* /* edxUnused */, int* pMovieClip, int contextId)
    {
        if (!hasBeenShiftedThisFrame)
        {
            HookCrashers::API::Client::LogInfo("First call this frame. Performing array shift...");

            void** ui_char_array = (void**)((char*)thisPtr + UI_CHAR_ARRAY_OFFSET);

            const int num_new_base_chars = CustomSave::CustomSaveManager::NUM_BASE_CHARACTERS - ORIGINAL_BASE_CHAR_COUNT;

            if (num_new_base_chars > 0)
            {
                HookCrashers::API::Client::LogInfo("Shifting workshop characters by " + std::to_string(num_new_base_chars) + " slots.");

                for (int i = CustomSaveManager::TOTAL_STREAMED_CHARACTERS - 1; i >= CustomSave::CustomSaveManager::NUM_BASE_CHARACTERS; --i)
                {
                    int source_index = i - num_new_base_chars;

                    if (source_index >= ORIGINAL_BASE_CHAR_COUNT && source_index < CustomSave::CustomSaveManager::TOTAL_STREAMED_CHARACTERS) {
                        ui_char_array[i] = ui_char_array[source_index];
                    }
                    else {
                        ui_char_array[i] = nullptr;
                    }
                }

                for (int i = 0; i < num_new_base_chars; ++i) {
                    int insertion_index = ORIGINAL_BASE_CHAR_COUNT + i;

                    if (ui_char_array[0] != nullptr) {
                        ui_char_array[insertion_index] = ui_char_array[0]; 
                    }
                }
                HookCrashers::API::Client::LogInfo("Inserted " + std::to_string(num_new_base_chars) + " placeholders for new base characters.");
            }

            hasBeenShiftedThisFrame = true;
        }

        if (g_originalFunction) {
            g_originalFunction(thisPtr, pMovieClip, contextId);
        }
    }

    void ResetGraphicShiftState() {
        if (hasBeenShiftedThisFrame) {
            HookCrashers::API::Client::LogInfo("[GRAPHIC HOOK] Resetting shift state.");
            hasBeenShiftedThisFrame = false;
        }
    }

    bool SetupAttachSkinGraphicHook(uintptr_t moduleBase) {
        HookCrashers::API::Client::LogInfo("Setting up AttachSkinGraphic hook...");

        uintptr_t targetAddress = moduleBase + AttachSkinGraphic_OFFSET;
        g_originalFunction = reinterpret_cast<OriginalAttachSkinGraphic_t>(targetAddress);

        if (!g_originalFunction) {
            HookCrashers::API::Client::LogError("Target address for AttachSkinGraphic is invalid.");
            return false;
        }

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        if (DetourAttach(&(PVOID&)g_originalFunction, DetouredAttachSkinGraphic) != NO_ERROR) {
            HookCrashers::API::Client::LogError("DetourAttach for AttachSkinGraphic failed");
            DetourTransactionAbort();
            return false;
        }
        if (DetourTransactionCommit() != NO_ERROR) {
            HookCrashers::API::Client::LogError("DetourTransactionCommit for AttachSkinGraphic failed");
            return false;
        }

        HookCrashers::API::Client::LogInfo("AttachSkinGraphic hook attached successfully!");
        return true;
    }
}