#include <windows.h>
#include <detours.h>
#include <cstdint>
#include <vector>
#include <new>
#include <HookCrashers.h>
#include <HookCrashers/Public/NativeCaller.h>
#include <HookCrashers/Public/NativeFunctions.h>

namespace CustomSave {
    using MainMenuBuilder_t = void* (__thiscall*)(void* this_ptr, void* param_2, void* param_3);
    using SetMenuState_t = void(__thiscall*)(void* this_ptr, uint16_t menu_id, void* pMenuObject);
    using LoadSymbol_t = void* (__thiscall*)(void* pMenuHandler, void* pSceneObject, void* pStringObject, uint32_t someID, float x, float y);
    using PrepareStringObject_t = char* (__thiscall*)(void* pStringObjectBuffer, const char* c_string);
    using FinalizeAndAddElement_t = void(__thiscall*)(void* pMenuHandler, void* pElement);
    using AllocateData_t = void* (__thiscall*)(void* this_ptr, uint32_t size);
    using CreateUIElement_t = void* (__thiscall*)(void*, void*, void*, uint32_t, uint32_t, uint8_t);
    using RenderScene_t = void(__fastcall*)(void* param_1);
    using IsInputPressed_t = bool(__thiscall*)(void* pMainMenu, short input_id);

    static MainMenuBuilder_t g_originalMainMenuBuilder = nullptr;
    static AllocateData_t g_originalAllocateData = nullptr;
    static SetMenuState_t SetMenuState = nullptr;
    static LoadSymbol_t LoadSymbol = nullptr;
    static PrepareStringObject_t PrepareStringObject = nullptr;
    static FinalizeAndAddElement_t FinalizeAndAddElement = nullptr;
    static CreateUIElement_t CreateUIElement = nullptr;
    static RenderScene_t g_originalRenderScene = nullptr;
    static IsInputPressed_t g_isLeftKeyPressed = nullptr;

    static void* g_pFirstTimeMenuHandler = nullptr;
    static void* g_pMemoryManager = nullptr;
    static void* g_pUnknownDispatcher = nullptr;
    static void* g_pButtonChoicesHandler = nullptr;
    static void* g_inputPtr = nullptr;
    static void* g_pFirstTimeSymbol = nullptr;

    // Aggiunte per il cleanup
    static void* g_pMainMenuInstance = nullptr;
    static char* g_pNewBigDataBlock = nullptr;
    static uintptr_t g_pOriginalBigDataBlock = 0;
    static bool g_bMenuHookActive = false;

    constexpr uintptr_t MAIN_MENU_BUILDER_OFFSET = 0x1420E0;
    constexpr uintptr_t SET_MENU_STATE_OFFSET = 0x12A2B0;
    constexpr uintptr_t LOAD_SYMBOL_OFFSET = 0x1534A0;
    constexpr uintptr_t PREPARE_STRING_OBJECT_OFFSET = 0x2EE80;
    constexpr uintptr_t FINALIZE_AND_ADD_ELEMENT_OFFSET = 0x1501C0;
    constexpr uintptr_t ALLOCATE_DATA_OFFSET = 0xDDDE0;
    constexpr uintptr_t CREATE_UI_ELEMENT = 0x151CC0;
    constexpr uintptr_t UNKNOWN_DISPATCHER_OFFSET = 0x144590;
    constexpr uintptr_t RENDER_SCENE_OFFSET = 0xF5730;

    constexpr uintptr_t RENDER_PTR_OFFSET_1 = 0x38E284;
    constexpr uintptr_t RENDER_PTR_OFFSET_2 = 0x230AC0;
    constexpr uintptr_t FRAME_OFFSET_1 = 0x35E160;
    static void* ptrRenderOffset1 = nullptr;
    static void* ptrRenderOffset2 = nullptr;
    static void* ptrFrameOffset1 = nullptr;
    static void* ptrFrameSelectDefault = nullptr;

    constexpr uintptr_t IS_LEFT_KEY_PRESSED_OFFSET = 0x14F340;

    constexpr size_t HANDLER_SIZE = 0x20C;
    constexpr size_t ORIGINAL_HANDLER_COUNT = 46;
    constexpr size_t NEW_HANDLER_COUNT = 47;
    constexpr size_t NEW_HANDLER_SIZE_BYTES = NEW_HANDLER_COUNT * HANDLER_SIZE;
    constexpr uint16_t FIRST_TIME_MENU_ID = ORIGINAL_HANDLER_COUNT;

    static int g_currentFirstTimeFrame = 0;

    // Forward declarations delle funzioni hook
    void* __fastcall HookedMainMenuBuilder(void* this_ptr, void* edx_dummy, void* param_2, void* param_3);
    void* __fastcall DetouredAllocateData(void* this_ptr, void* edx_dummy, uint32_t size);
    void __fastcall DetouredRenderScene(void* param_1);
    bool __fastcall HookedIsLeftKeyPressed(void* this_ptr, void* edx_dummy, short param_1);

    // Funzione per ripristinare lo stato originale del menu
    void RestoreOriginalMenuState() {
        if (g_pMainMenuInstance && g_pOriginalBigDataBlock && g_bMenuHookActive) {
            HookCrashers::LogInfo("[FirstTimeMenu] Restoring original menu state...");

            // Ripristina il puntatore originale
            *(uintptr_t*)((uintptr_t)g_pMainMenuInstance + 0x40) = g_pOriginalBigDataBlock;

            // Libera la memoria allocata
            if (g_pNewBigDataBlock) {
                delete[] g_pNewBigDataBlock;
                g_pNewBigDataBlock = nullptr;
            }

            g_bMenuHookActive = false;
            HookCrashers::LogInfo("[FirstTimeMenu] Original menu state restored successfully.");
        }
    }

    // Funzione per rimuovere completamente gli hook
    void RemoveAllHooks() {
        HookCrashers::LogInfo("[FirstTimeMenu] Removing all hooks...");

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        if (g_originalMainMenuBuilder) {
            DetourDetach(&(PVOID&)g_originalMainMenuBuilder, HookedMainMenuBuilder);
        }

        if (g_originalAllocateData) {
            DetourDetach(&(PVOID&)g_originalAllocateData, DetouredAllocateData);
        }

        if (g_originalRenderScene) {
            DetourDetach(&(PVOID&)g_originalRenderScene, DetouredRenderScene);
        }

        if (g_isLeftKeyPressed) {
            DetourDetach(&(PVOID&)g_isLeftKeyPressed, HookedIsLeftKeyPressed);
        }

        DetourTransactionCommit();
        HookCrashers::LogInfo("[FirstTimeMenu] All hooks removed successfully.");
    }

    void* __fastcall DetouredAllocateData(void* this_ptr, void* edx_dummy, uint32_t size) {
        if (!g_pMemoryManager) {
            g_pMemoryManager = this_ptr;
        }
        return g_originalAllocateData(this_ptr, size);
    }

    void __fastcall DetouredRenderScene(void* param_1) {
        g_originalRenderScene(param_1);
    }

    void __cdecl FirstTime_OnAccept(void* pMainMenu) {
        HookCrashers::LogInfo("[FirstTimeMenu] User clicked ACCEPT.");

        // Esegui le operazioni di salvataggio
        //CustomSave::CustomSaveManager::getInstance().triggerOriginalSaveImport();
        //CustomSave::CustomSaveManager::getInstance().markFirstTimeSetupAsComplete();

        // Ripristina lo stato originale del menu PRIMA di cambiare stato
        RestoreOriginalMenuState();

        // Cambia stato del menu
        SetMenuState(pMainMenu, 0x17, g_pButtonChoicesHandler);

        // Rimuovi gli hook per evitare problemi futuri
        RemoveAllHooks();
    }

    void __cdecl FirstTime_OnDecline(void* pMainMenu) {
        HookCrashers::LogInfo("[FirstTimeMenu] User clicked DECLINE.");

        // Esegui le operazioni di salvataggio
        //CustomSave::CustomSaveManager::getInstance().declineOriginalSaveImport();
        //CustomSave::CustomSaveManager::getInstance().markFirstTimeSetupAsComplete();

        // Ripristina lo stato originale del menu PRIMA di cambiare stato
        RestoreOriginalMenuState();

        // Cambia stato del menu
        SetMenuState(pMainMenu, 0x17, g_pButtonChoicesHandler);

        // Rimuovi gli hook per evitare problemi futuri
        RemoveAllHooks();
    }

    void __cdecl FirstTime_OnSelectAccept(void* pMainMenu) {
    }

    void __cdecl FirstTime_OnSelectDecline(void* pMainMenu) {
    }

    void __cdecl FirstTime_InitializeFrame(void* pHandlerData) {
        if (!pHandlerData) {
            return;
        }

        // Ottieni il puntatore alla scena
        void* pScene = *(void**)((uintptr_t)pHandlerData + 0x38);
        if (!pScene) {
            return;
        }

        // Naviga alla struttura del menu object
        void* pMenuHandler = *(void**)((uintptr_t)pScene + 0x148);
        if (!pMenuHandler) {
            return;
        }

        void* pMenuObject = *(void**)((uintptr_t)pMenuHandler + 0x54);
        if (!pMenuObject) {
            return;
        }

        // Ottieni il frame corrente e calcola il prossimo
        uint16_t currentFrame = *(uint16_t*)((uintptr_t)pScene + 0x0C);
        uint16_t targetFrame = currentFrame + 1;

        if (targetFrame != 0) {
            // Verifica il numero massimo di frame disponibili
            uint16_t maxFrames = *(uint16_t*)((uintptr_t)pMenuObject + 0xF4);

            if (maxFrames < targetFrame) {
                targetFrame = maxFrames;
            }

            // Imposta il flag per il cambio frame
            uint8_t* pFlags = (uint8_t*)((uintptr_t)pMenuObject + 0xEC);
            *pFlags = *pFlags | 0x40;

            // Effettua il cambio frame iniziale
            HookCrashers::Native::CallNative(HookCrashers::Native::Natives::GotoAndStop, pMenuObject, targetFrame);
        }
    }

    void __cdecl FirstTime_OnUpdate(void* pHandlerData) {
        void* pScene = *(void**)((uintptr_t)pHandlerData + 0x38);
        if (!pScene) {
            return;
        }

        // Naviga alla struttura del menu object
        void* pMenuHandler = *(void**)((uintptr_t)pScene + 0x148);
        if (!pMenuHandler) {
            return;
        }

        void* pMenuObject = *(void**)((uintptr_t)pMenuHandler + 0x54);
        if (!pMenuObject) {
            return;
        }

        void* pInputFlag = *(void**)((uintptr_t)pScene + 0x10);

        uint16_t currentFrame = *(uint16_t*)((uintptr_t)pScene + 0x0C);
        uint16_t newFrame = currentFrame;
        if (HookCrashers::Native::CallNative<bool>(HookCrashers::Native::Natives::IsDownPressed, g_inputPtr, (short)-2) ||
            HookCrashers::Native::CallNative<bool>(HookCrashers::Native::Natives::IsUpPressed, g_inputPtr, (short)-2)) {
            newFrame = currentFrame == 1 ? 0 : 1;
        }

        if (HookCrashers::Native::CallNative<bool>(HookCrashers::Native::Natives::IsEnterPressed, g_inputPtr, (short)-2)) {
            HookCrashers::Native::CallNative(HookCrashers::Native::Natives::RefreshGui, 1, 0);
            if (newFrame == 0) {
                FirstTime_OnAccept(pHandlerData);
            }
            else {
                FirstTime_OnDecline(pHandlerData);
            }
        }

        if (newFrame != currentFrame) {
            *(uint16_t*)((uintptr_t)pScene + 0x0C) = newFrame;
            HookCrashers::Native::CallNative(HookCrashers::Native::Natives::GotoAndStop, pMenuObject, newFrame + 1);
        }
    }

    void __fastcall BuildFirstTimeMenu(void* this_scene, void* edx_dummy, void* pHandlerData) {
        if (!g_pMemoryManager || !g_originalAllocateData) {
            return;
        }

        void* pSceneObject = *(void**)((uintptr_t)this_scene + 4);

        const uint32_t symbolObjectSize = 0x5C;
        void* pMemoryForSymbol = g_originalAllocateData(g_pMemoryManager, symbolObjectSize);

        if (!pMemoryForSymbol) {
            return;
        }

        uint8_t tempStringObj[0x40];
        PrepareStringObject(tempStringObj, "firsttime");

        void* pFirstTime = LoadSymbol(pMemoryForSymbol, pSceneObject, tempStringObj, 10000, 0.0f, 0.0f);

        if (pFirstTime) {
            g_pFirstTimeSymbol = pFirstTime;
            FinalizeAndAddElement(pHandlerData, pFirstTime);
            *(uintptr_t*)((uintptr_t)pHandlerData + 0x148) = (uintptr_t)pFirstTime;
        }
        else {
            return;
        }

        /*uint16_t* pListCounter = (uint16_t*)((uintptr_t)pHandlerData + 0x0A);
        void** pButtonList = (void**)((uintptr_t)pHandlerData + 0x1C);

        // Create visual UI elements for input handling
        void* acceptButton = g_originalAllocateData(g_pMemoryManager, 0x50);
        CreateUIElement(acceptButton, this_scene, nullptr, 0, 0, 0);
        *(uint16_t*)((uintptr_t)acceptButton + 0x48) = 1;
        FinalizeAndAddElement(pHandlerData, acceptButton);

        // Hotspot 1: Enter - context-sensitive based on current frame
        void* acceptHotspot = g_originalAllocateData(g_pMemoryManager, 0x34);
        memset(acceptHotspot, 0, 0x34);
        *(uint8_t*)((uintptr_t)acceptHotspot + 0x30) |= 1;

        uint32_t* acceptData = (uint32_t*)acceptHotspot;
        float accept_x1 = 25.5f, accept_y1 = 91.5f, accept_x2 = 278.5f, accept_y2 = 144.5f;
        acceptData[0] = *reinterpret_cast<uint32_t*>(&accept_x1);
        acceptData[1] = *reinterpret_cast<uint32_t*>(&accept_y1);
        acceptData[2] = *reinterpret_cast<uint32_t*>(&accept_x2);
        acceptData[3] = *reinterpret_cast<uint32_t*>(&accept_y2);
        acceptData[7] = (uint32_t)this_scene;
        acceptData[8] = (uint32_t)&FirstTime_OnSelectAccept;
        acceptData[10] = (uint32_t)&FirstTime_OnClickAccept;

        if (*pListCounter < 40) {
            pButtonList[*pListCounter] = acceptHotspot;
            (*pListCounter)++;
        }

        void* declineButton = g_originalAllocateData(g_pMemoryManager, 0x50);
        CreateUIElement(declineButton, this_scene, nullptr, 0, 0, 0);
        *(uint16_t*)((uintptr_t)declineButton + 0x48) = 1;
        FinalizeAndAddElement(pHandlerData, declineButton);

        void* declineHotspot = g_originalAllocateData(g_pMemoryManager, 0x34);
        memset(declineHotspot, 0, 0x34);
        *(uint8_t*)((uintptr_t)declineHotspot + 0x30) |= 1;

        uint32_t* declineData = (uint32_t*)declineHotspot;
        float decline_x1 = 150.0f, decline_y1 = 150.0f, decline_x2 = 250.0f, decline_y2 = 250.0f;
        declineData[0] = *reinterpret_cast<uint32_t*>(&decline_x1);
        declineData[1] = *reinterpret_cast<uint32_t*>(&decline_y1);
        declineData[2] = *reinterpret_cast<uint32_t*>(&decline_x2);
        declineData[3] = *reinterpret_cast<uint32_t*>(&decline_y2);
        declineData[7] = (uint32_t)this_scene;
        declineData[8] = (uint32_t)&FirstTime_OnSelectDecline;
        declineData[10] = (uint32_t)&FirstTime_OnClickDecline;

        if (*pListCounter < 40) {
            pButtonList[*pListCounter] = declineHotspot;
            (*pListCounter)++;
        }*/

        *(uintptr_t*)((uintptr_t)pHandlerData + 0x1AC) = (uintptr_t)this_scene;
        *(void**)((uintptr_t)pHandlerData + 0x1D0) = (void*)&FirstTime_InitializeFrame;
        *(void**)((uintptr_t)pHandlerData + 0x1C8) = (void*)&FirstTime_OnUpdate;

        *(uint16_t*)((uintptr_t)pHandlerData + 0x0C) = 0;
        void** pFirstButton = (void**)((uintptr_t)pHandlerData + 0x44);
        if (*pFirstButton) {
            *(uint32_t*)((uintptr_t)*pFirstButton + 0x10) = 0xffff00ff;
        }
        *(uint8_t*)((uintptr_t)pHandlerData + 0x14) = 1;
    }

    bool __fastcall HookedIsLeftKeyPressed(void* this_ptr, void* edx_dummy, short param_1) {
        if (g_isLeftKeyPressed) {
            if (g_inputPtr == nullptr || g_inputPtr != this_ptr) {
                g_inputPtr = this_ptr;
            }
            return g_isLeftKeyPressed(this_ptr, param_1);
        }

        return false;
    }

    void* __fastcall HookedMainMenuBuilder(void* this_ptr, void* edx_dummy, void* param_2, void* param_3)
    {
        void* result = g_originalMainMenuBuilder(this_ptr, param_2, param_3);

        // Salva l'istanza del menu principale per il cleanup
        g_pMainMenuInstance = this_ptr;

        uintptr_t pOriginalBigDataBlock = *(uintptr_t*)((uintptr_t)this_ptr + 0x40);
        if (!pOriginalBigDataBlock) {
            return result;
        }

        // Salva il puntatore originale per il restore
        g_pOriginalBigDataBlock = pOriginalBigDataBlock;

        char* pNewBigDataBlock = new (std::nothrow) char[NEW_HANDLER_SIZE_BYTES];
        if (!pNewBigDataBlock) {
            return result;
        }

        // Salva il puntatore del nuovo buffer per il cleanup
        g_pNewBigDataBlock = pNewBigDataBlock;

        const size_t originalBlockSize = ORIGINAL_HANDLER_COUNT * HANDLER_SIZE;
        memcpy(pNewBigDataBlock, (void*)pOriginalBigDataBlock, originalBlockSize);

        void* pOurHandlerSlot = pNewBigDataBlock + originalBlockSize;
        void* pTemplateHandler = (void*)pOriginalBigDataBlock;

        memcpy(pOurHandlerSlot, pTemplateHandler, HANDLER_SIZE);

        BuildFirstTimeMenu(this_ptr, nullptr, pOurHandlerSlot);

        *(uintptr_t*)((uintptr_t)this_ptr + 0x40) = (uintptr_t)pNewBigDataBlock;
        g_bMenuHookActive = true;

        /*if (CustomSave::CustomSaveManager::getInstance().isFirstTimeSetupNeeded()) {
            SetMenuState(this_ptr, FIRST_TIME_MENU_ID, pOurHandlerSlot);
        }*/

        // Save buttonchoices handler to use later
        g_pButtonChoicesHandler = (void*)(pOriginalBigDataBlock + (0x17 * HANDLER_SIZE));
        return result;
    }

    bool SetupMainMenuBuilderHook(uintptr_t moduleBase)
    {
        g_originalMainMenuBuilder = reinterpret_cast<MainMenuBuilder_t>(moduleBase + MAIN_MENU_BUILDER_OFFSET);
        SetMenuState = reinterpret_cast<SetMenuState_t>(moduleBase + SET_MENU_STATE_OFFSET);
        LoadSymbol = reinterpret_cast<LoadSymbol_t>(moduleBase + LOAD_SYMBOL_OFFSET);
        PrepareStringObject = reinterpret_cast<PrepareStringObject_t>(moduleBase + PREPARE_STRING_OBJECT_OFFSET);
        FinalizeAndAddElement = reinterpret_cast<FinalizeAndAddElement_t>(moduleBase + FINALIZE_AND_ADD_ELEMENT_OFFSET);
        CreateUIElement = reinterpret_cast<CreateUIElement_t>(moduleBase + CREATE_UI_ELEMENT);
        g_pUnknownDispatcher = reinterpret_cast<void*>(moduleBase + UNKNOWN_DISPATCHER_OFFSET);

        ptrRenderOffset1 = reinterpret_cast<void*>(moduleBase + RENDER_PTR_OFFSET_1);
        ptrRenderOffset2 = reinterpret_cast<void*>(moduleBase + RENDER_PTR_OFFSET_2);
        ptrFrameOffset1 = reinterpret_cast<void*>(moduleBase + FRAME_OFFSET_1);
        ptrFrameSelectDefault = reinterpret_cast<void*>(moduleBase + 0x158810);

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        g_isLeftKeyPressed = reinterpret_cast<IsInputPressed_t>(moduleBase + IS_LEFT_KEY_PRESSED_OFFSET);
        LONG error = DetourAttach(&(PVOID&)g_isLeftKeyPressed, HookedIsLeftKeyPressed);
        if (error != NO_ERROR) {
            DetourTransactionAbort();
            return false;
        }

        g_originalAllocateData = reinterpret_cast<AllocateData_t>(moduleBase + ALLOCATE_DATA_OFFSET);
        error = DetourAttach(&(PVOID&)g_originalAllocateData, DetouredAllocateData);
        if (error != NO_ERROR) {
            DetourTransactionAbort();
            return false;
        }

        g_originalRenderScene = reinterpret_cast<RenderScene_t>(moduleBase + RENDER_SCENE_OFFSET);
        error = DetourAttach(&(PVOID&)g_originalRenderScene, DetouredRenderScene);
        if (error != NO_ERROR) {
            DetourTransactionAbort();
            return false;
        }

        if (DetourAttach(&(PVOID&)g_originalMainMenuBuilder, HookedMainMenuBuilder) != NO_ERROR) {
            DetourTransactionAbort();
            return false;
        }

        if (DetourTransactionCommit() != NO_ERROR) {
            return false;
        }

        return true;
    }

    // Funzione opzionale per cleanup manuale (da chiamare durante il shutdown del modulo)
    void CleanupFirstTimeMenu() {
        RestoreOriginalMenuState();
        RemoveAllHooks();

        // Reset di tutte le variabili globali
        g_pMainMenuInstance = nullptr;
        g_pOriginalBigDataBlock = 0;
        g_bMenuHookActive = false;
        g_pFirstTimeSymbol = nullptr;
        g_inputPtr = nullptr;
    }
}