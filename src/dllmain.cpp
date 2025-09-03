#include "stdafx.h"
#include "Core/CustomSaveManager.h"
#include "Overrides/RegisterOverrides.h"
#include "Hooks/AttachSkinGraphicHook.h"
#include "Hooks/InitCharDataTableHook.h"
#include "Hooks/MainMenuBuilderHook.h"
#include <HookCrashers.h> // L'unico header che ti serve
#include <windows.h>

// La callback ora usa le nuove classi helper per chiarezza e sicurezza.
static void GetCustomSaveDataHandler(int paramCount, HC_SWFArgument** swfArgs, HC_SWFReturn* swfReturn) {
    HookCrashers::SWF::ArgsReader args(paramCount, swfArgs);
    HookCrashers::SWF::ReturnValue ret(swfReturn);

    if (paramCount < 1) {
        ret.SetFailure();
        return;
    }

    std::string property = args.GetString(0);
    int returnValue = -1;

    if (property == "char_offset") returnValue = CustomSave::CustomSaveManager::NUM_GLOBAL_BYTES;
    else if (property == "char_size") returnValue = CustomSave::CustomSaveManager::NUM_CHARACTER_BYTES;
    else if (property == "num_items") returnValue = 128;
    else if (property == "num_animals") returnValue = 32;
    else if (property == "num_levels") returnValue = 64;
    else if (property == "num_relics") returnValue = 8;
    else if (property == "num_items_expansion") returnValue = 64;
    else if (property == "num_characters_legacy") returnValue = CustomSave::CustomSaveManager::NUM_BASE_CHARACTERS_LEGACY;
    else if (property == "num_characters_noaddons") returnValue = CustomSave::CustomSaveManager::NUM_BASE_CHARACTERS;
    else if (property == "num_characters_safe") returnValue = CustomSave::CustomSaveManager::TOTAL_GAME_CHARACTERS;
    else if (property == "num_characters_addons") returnValue = CustomSave::CustomSaveManager::NUM_WORKSHOP_CHARACTERS;
    else if (property == "num_characters") returnValue = CustomSave::CustomSaveManager::TOTAL_STREAMED_CHARACTERS;

	HookCrashers::LogInfo("[GetCustomSaveData] Property requested: " + property + " => " + std::to_string(returnValue));

    ret.SetInt(returnValue);
}

extern "C" {
    // Queste esportazioni rimangono invariate, sono per il mod loader
    __declspec(dllexport) const char* GetModName() { return "Custom Save"; }
    __declspec(dllexport) const char* GetModAuthor() { return "ilVonBurza"; }
    __declspec(dllexport) const char* GetModVersion() { return "3.0"; }

    // La funzione di inizializzazione ora usa le chiamate dirette di HookCrashers
    __declspec(dllexport) bool InitializeMod() {
        // La chiamata a HookCrashers::Initialize() dovrebbe essere fatta dal Mod Loader stesso.
        // Se non lo fa, dovresti aggiungerla qui. Assumiamo che il loader la gestisca.
        // if (!HookCrashers::IsInitialized()) { /* Errore, HookCrashers non è caricato */ return false; }

        HookCrashers::LogInfo("[CustomSave] Initializing via Mod Loader...");

        char dllPath[MAX_PATH] = { 0 };
        HMODULE hMod = GetModuleHandleA("CustomSave.asi");
        if (!hMod) {
            HookCrashers::LogError("[CustomSave] Failed to get module handle for CustomSave.asi!");
            return false;
        }
        GetModuleFileNameA(hMod, dllPath, MAX_PATH);

        std::string modFullPath = dllPath;
        size_t lastSlash = modFullPath.find_last_of("\\/");
        std::string modDirectory = (lastSlash != std::string::npos) ? modFullPath.substr(0, lastSlash + 1) : "";

        if (modDirectory.empty()) {
            HookCrashers::LogError("[CustomSave] Could not determine mod directory path!");
            return false;
        }

        std::string saveFilePath = modDirectory;
        HookCrashers::LogInfo("[CustomSave] Save file path set to: " + saveFilePath);

        if (!CustomSave::CustomSaveManager::getInstance().initialize(saveFilePath)) {
            HookCrashers::LogError("[CustomSave] CustomSaveManager initialization failed!");
            return false;
        }

        // Chiamate alle funzioni di setup
        CustomSave::RegisterStorageOverrides();

        uintptr_t moduleBase = HookCrashers::GetModuleBase();
        CustomSave::SetupAttachSkinGraphicHook(moduleBase);
        CustomSave::SetupInitCharDataTableHook(moduleBase);
        if (CustomSave::CustomSaveManager::getInstance().isFirstTimeSetupNeeded()) {
            CustomSave::SetupMainMenuBuilderHook(moduleBase); // This crashes
        }

        // Registrazione della funzione custom SWF
        HookCrashers::RegisterCustomSWF(50100, "GetCustomSaveData", GetCustomSaveDataHandler);

        HookCrashers::LogInfo("[CustomSave] Successfully initialized.");
        return true;
    }
}

// DllMain rimane invariato
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}