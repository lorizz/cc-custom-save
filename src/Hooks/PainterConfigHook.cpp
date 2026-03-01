#include "PainterConfigHook.h"
#include <windows.h>
#include <detours.h>
#include <malloc.h>
#include <iostream>
#include <HookCrashers.h>

namespace CustomSave {

    static const uintptr_t RVA_PAINTER_ENTRY = 0x5C080;
    static const uintptr_t RVA_XML_INIT = 0xA2A20;
    static const uintptr_t RVA_XML_LOAD = 0xA2F60;
    static const uintptr_t RVA_XML_PARSE = 0x5C150;
    static const uintptr_t RVA_XML_CLEAN = 0xA2C60;
    static const uintptr_t RVA_UNK_GLOBAL = 0x261C0;
    static const uintptr_t RVA_UNK_VAR = 0x1C24B3;

    typedef void(__thiscall* tXML_Init) (void* pThis, int a2);
    typedef int(__thiscall* tXML_Load) (void* pThis, int pFileData);
    typedef char(__thiscall* tXML_Parse)(void* pBufferAsThis, int a2);
    typedef void(__thiscall* tXML_Clean)(void* pThis);
    typedef void* (__thiscall* tUnk_Func)(void* pThis, const char* s);
    typedef char(__thiscall* tPainterEntry)(void* pThis, int pFileData);

    static tPainterEntry g_origPainterEntry = nullptr;

    static char __cdecl HookedPainterConfig_Impl(void* pThis, int pFileData)
    {
        uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
        uint8_t* doc = (uint8_t*)pThis;

        auto XML_Init = (tXML_Init)(base + RVA_XML_INIT);
        auto XML_Load = (tXML_Load)(base + RVA_XML_LOAD);
        auto XML_Parse = (tXML_Parse)(base + RVA_XML_PARSE);
        auto XML_Clean = (tXML_Clean)(base + RVA_XML_CLEAN);
        auto Unk_Func = (tUnk_Func)(base + RVA_UNK_GLOBAL);

        doc[84] = 1;

        HookCrashers::LogDebug("Starting pFileData check");
        if (pFileData)
        {
            XML_Init(pThis, 0);
            HookCrashers::LogDebug("XML Initialized");

            int loadResult = XML_Load(pThis, pFileData);
            HookCrashers::LogDebug("Result loaded");

            // 64KB sulla heap, supporta un numero molto alto di addon
            const size_t BUFFER_SIZE = 1024 * 1024;
            void* heapBuffer = malloc(BUFFER_SIZE);
            if (!heapBuffer)
            {
                HookCrashers::LogError("Failed to allocate heap buffer!");
                XML_Clean(pThis);
                return 0;
            }
            memset(heapBuffer, 0, BUFFER_SIZE);
            HookCrashers::LogDebug("Allocated extended buffer (64KB heap)");

            char parseResult = XML_Parse(heapBuffer, (int)pThis);
            HookCrashers::LogDebug("Parsed result");

            free(heapBuffer);

            if (loadResult == 0 && parseResult)
            {
                HookCrashers::LogDebug("Cleaning xml");
                XML_Clean(pThis);
                doc[84] = 0;
                HookCrashers::LogDebug("Cleaned");

                const char* emptyStr = (const char*)(base + RVA_UNK_VAR);
                Unk_Func(pThis, emptyStr);
                HookCrashers::LogDebug("Assigned string");
                return 1;
            }

            XML_Clean(pThis);
        }

        return 0;
    }

    __declspec(naked) static char __stdcall HookedPainterConfig_Naked(int pFileData)
    {
        __asm {
            push  pFileData
            push  ecx
            call  HookedPainterConfig_Impl
            add   esp, 8
            ret   4
        }
    }

    bool SetupPainterConfigHook() {
        uintptr_t base = (uintptr_t)GetModuleHandle(NULL);
        g_origPainterEntry = (tPainterEntry)(base + RVA_PAINTER_ENTRY);

        if (!g_origPainterEntry) return false;

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)g_origPainterEntry, HookedPainterConfig_Naked);

        if (DetourTransactionCommit() != NO_ERROR) {
            HookCrashers::LogError("[PainterConfig] Detour FAILED");
            return false;
        }

        HookCrashers::LogInfo("[PainterConfig] Hook installed successfully (64KB heap buffer)");
        return true;
    }
}