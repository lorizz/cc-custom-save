#include "AttachSkinGraphicHook.h"
#include <HookCrashers.h> 
#include <detours.h>
#include <windows.h>
#include "../Core/CustomSaveManager.h"

// --- Costanti ---
constexpr int ORIGINAL_BASE_CHAR_COUNT = 32;
constexpr int WORKSHOP_CHAR_COUNT = 10;
constexpr uintptr_t UI_CHAR_ARRAY_OFFSET = 0x11A8;

namespace CustomSave {
    constexpr uintptr_t AttachSkinGraphic_OFFSET = 0x8C820;
    using OriginalAttachSkinGraphic_t = void(__thiscall*)(void* thisPtr, int* pMovieClip, int contextId);
    static OriginalAttachSkinGraphic_t g_originalFunction = nullptr;
    bool hasBeenShiftedThisFrame = false;

    void __fastcall DetouredAttachSkinGraphic(void* thisPtr, void* /* edxUnused */, int* pMovieClip, int contextId)
    {
        HookCrashers::LogInfo("[GRAPHIC HOOK] DetouredAttachSkinGraphic called.");

        if (!hasBeenShiftedThisFrame)
        {
            HookCrashers::LogInfo("First call this frame. Performing array shift...");
            void** ui_char_array = (void**)((char*)thisPtr + UI_CHAR_ARRAY_OFFSET);
            const int num_addon_chars = CustomSave::CustomSaveManager::NUM_BASE_CHARACTERS - ORIGINAL_BASE_CHAR_COUNT;

            if (num_addon_chars > 0)
            {
                HookCrashers::LogInfo("Shifting workshop characters by " + std::to_string(num_addon_chars) + " slots.");

                // Posizioni:
                // - Base characters: 0-31 (rimangono invariati)
                // - Addon characters: 32 a (32 + num_addon_chars - 1) 
                // - Workshop characters: da 32 a (32 + num_addon_chars + 9)

                const int old_workshop_start = ORIGINAL_BASE_CHAR_COUNT; // 32
                const int new_workshop_start = ORIGINAL_BASE_CHAR_COUNT + num_addon_chars; // 32 + addon

                // Shifta i 10 workshop characters dalla posizione 32 alla posizione 32+addon
                // IMPORTANTE: shifta dall'ultimo al primo per evitare sovrapposizioni
                for (int i = WORKSHOP_CHAR_COUNT - 1; i >= 0; --i)
                {
                    int old_index = old_workshop_start + i;  // da 32 a 41
                    int new_index = new_workshop_start + i;  // da (32+addon) a (41+addon)

                    HookCrashers::LogInfo("Moving workshop char from index " + std::to_string(old_index) +
                        " to " + std::to_string(new_index));

                    // Copia il workshop character alla nuova posizione
                    ui_char_array[new_index] = ui_char_array[old_index];

                    // Cancella la vecchia posizione solo se è diversa dalla nuova
                    if (old_index != new_index) {
                        ui_char_array[old_index] = nullptr;
                    }
                }

                // Ora inserisci i placeholders per i nuovi addon characters
                for (int i = 0; i < num_addon_chars; ++i) {
                    int addon_index = ORIGINAL_BASE_CHAR_COUNT + i; // 32, 33, 34, ...

                    HookCrashers::LogInfo("Setting addon placeholder at index " + std::to_string(addon_index));

                    // Usa il primo character come placeholder
                    if (ui_char_array[0] != nullptr) {
                        ui_char_array[addon_index] = ui_char_array[0];
                    }
                    else {
                        ui_char_array[addon_index] = nullptr;
                    }
                }

                HookCrashers::LogInfo("Shift completed. Addon chars: " + std::to_string(num_addon_chars) +
                    ", Workshop chars moved to start at index: " + std::to_string(new_workshop_start));

                // Debug: stampa lo stato dell'array dopo lo shift
                for (int i = 30; i < 50 && i < 72; ++i) {
                    HookCrashers::LogInfo("ui_char_array[" + std::to_string(i) + "] = " +
                        (ui_char_array[i] ? "valid" : "nullptr"));
                }
            }
            hasBeenShiftedThisFrame = true;
        }

        if (g_originalFunction) {
            g_originalFunction(thisPtr, pMovieClip, contextId);
        }
    }

    void ResetGraphicShiftState() {
        if (hasBeenShiftedThisFrame) {
            HookCrashers::LogInfo("[GRAPHIC HOOK] Resetting shift state.");
            hasBeenShiftedThisFrame = false;
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