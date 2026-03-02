#include "stdafx.h"
#include "Patcher/Patches.h"
#include "Core/AddonCharacterConfig.h"
#include <HookCrashers.h>
#include <windows.h>
#include "Hooks/PainterConfigHook.h"
#include "Hooks/GenerateDefaultCharacterDataHook.h"

static void GetCustomSaveDataHandler(int paramCount, HC_SWFArgument** swfArgs, HC_SWFReturn* swfReturn) {
    HookCrashers::SWF::ArgsReader args(paramCount, swfArgs);
    HookCrashers::SWF::ReturnValue ret(swfReturn);

    if (paramCount < 1) {
        ret.SetFailure();
        return;
    }

    std::string property = args.GetString(0);
    int returnValue = -1;

    if (property == "char_offset") returnValue = 64;
    else if (property == "char_size") returnValue = 48;
    else if (property == "num_items") returnValue = 128;
    else if (property == "num_animals") returnValue = 32;
    else if (property == "num_levels") returnValue = 64;
    else if (property == "num_relics") returnValue = 8;
    else if (property == "num_items_expansion") returnValue = 64;
    else if (property == "num_characters_legacy") returnValue = 31; // Vanilla classic => 31
    else if (property == "num_characters_noaddons") returnValue = 32 + AddonCharacterConfig::getInstance().getAddonCount(); // Vanilla fresh => 32
    else if (property == "num_characters_safe") returnValue = 42 + AddonCharacterConfig::getInstance().getAddonCount(); // Host => 32 + 10
    else if (property == "num_characters_addons") returnValue = 10; // Workshop characters
    else if (property == "num_characters") returnValue = 72 + AddonCharacterConfig::getInstance().getAddonCount(); // 32 + (10 * 4)

    HookCrashers::LogInfo("[CustomSave] GetCustomSaveData called! " + property + " " + std::to_string(returnValue));

    ret.SetInt(returnValue);
}

extern "C" {
    __declspec(dllexport) const char* GetModName() { return "Custom Save"; }
    __declspec(dllexport) const char* GetModAuthor() { return "ilVonBurza"; }
    __declspec(dllexport) const char* GetModVersion() { return "4.0"; }

    __declspec(dllexport) bool InitializeMod() {
        HookCrashers::LogInfo("[CustomSave] Initializing...");

        char dllPath[MAX_PATH] = { 0 };
        HMODULE hMod = GetModuleHandleA("CustomSave.asi");
        if (!hMod) {
            HookCrashers::LogError("[CustomSave] Failed to get module handle!");
            return false;
        }
        GetModuleFileNameA(hMod, dllPath, MAX_PATH);
        std::string modDir = dllPath;
        modDir = modDir.substr(0, modDir.find_last_of("\\/") + 1);

        std::string iniPath = modDir + "CustomSave.ini";
        AddonCharacterConfig::getInstance().loadFromIni(iniPath);
        HookCrashers::LogInfo("[CustomSave] Loaded " + std::to_string(AddonCharacterConfig::getInstance().getAddonCount()) + " addon characters.");

        CustomSave::ApplyPatches();
        CustomSave::SetupPainterConfigHook();
        CustomSave::SetupGenerateDefaultCharacterDataHook();

        HookCrashers::RegisterCustomSWF(50100, "GetCustomSaveData", GetCustomSaveDataHandler);

        HookCrashers::LogInfo("[CustomSave] Done.");
        return true;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(hModule);
    return TRUE;
}