#include "RegisterOverrides.h"
#include "../Core/CustomSaveManager.h"
#include <HookCrashers.h> // L'UNICO header che ti serve ora!
#include <HookCrashers/Public/NativeCaller.h>
#include <HookCrashers/Public/NativeFunctions.h>

// Le definizioni delle variabili statiche sono state rimosse perché
// la nuova API non ne ha bisogno in questo file. La gestione
// delle callback è interna alla DLL.

// SWF Function IDs (rimangono invariate)
#define SWF_ID_READ_STORAGE                     0x3F 
#define SWF_ID_WRITE_STORAGE                    0x40
#define SWF_ID_IS_CHARACTER_UNLOCKED_FOR_PLAYER 0xC4
#define SWF_ID_LOBBY_TRY_SELECT_CHAR            0xE5
#define SWF_ID_LOBBY_SKIN_CHAR_AVAIL            0xE6
#define SWF_ID_LOBBY_TRY_READY_SKINS            0xE7

// Assumiamo che le funzioni native siano definite altrove
namespace Natives = HookCrashers::Native::Natives;

// Static offsets per le letture/scritture sequenziali (logica interna)
static size_t s_current_read_offset = 0;
static size_t s_current_write_offset = 0;

namespace CustomSave {

    void ReadStorageOverride(void* thisPtr, int swfContext, uint32_t functionIdRaw, int paramCount, HC_SWFArgument** swfArgs, HC_SWFReturn* swfReturn, uint32_t callbackPtr) {
        HookCrashers::SWF::ArgsReader args(paramCount, swfArgs);
        HookCrashers::SWF::ReturnValue ret(swfReturn);
        CustomSaveManager& saveManager = CustomSaveManager::getInstance();

        uint8_t byte_value = 0;
        int playerPort = args.GetInt(0);

        if (paramCount == 2) { // (playerPort, offset)
            int new_offset = args.GetInt(1);
            s_current_read_offset = new_offset;
            byte_value = saveManager.readByteFromGameBuffer(s_current_read_offset);
            s_current_read_offset++;
        }
        else if (paramCount == 1) { // (playerPort) - sequential read
            byte_value = saveManager.readByteFromGameBuffer(s_current_read_offset);
            s_current_read_offset++;
        }
        else {
            HookCrashers::LogError("[CustomSave][ReadStorage] Unexpected number of parameters: " + std::to_string(paramCount));
        }

        ret.SetInt(byte_value);
    }

    void WriteStorageOverride(void* thisPtr, int swfContext, uint32_t functionIdRaw, int paramCount, HC_SWFArgument** swfArgs, HC_SWFReturn* swfReturn, uint32_t callbackPtr) {
        HookCrashers::SWF::ArgsReader args(paramCount, swfArgs);
        HookCrashers::SWF::ReturnValue ret(swfReturn);
        CustomSaveManager& saveManager = CustomSaveManager::getInstance();

        bool success = false;
        int playerPort = args.GetInt(0);

        if (paramCount == 3) { // (playerPort, offset, value)
            int new_offset = args.GetInt(1);
            int value = args.GetInt(2);
            s_current_write_offset = new_offset;
            saveManager.writeByteToGameBuffer(s_current_write_offset, static_cast<uint8_t>(value));
            s_current_write_offset++;
            success = true;
        }
        else if (paramCount == 2) { // (playerPort, value) - sequential write
            int value = args.GetInt(1);
            saveManager.writeByteToGameBuffer(s_current_write_offset, static_cast<uint8_t>(value));
            s_current_write_offset++;
            success = true;
        }
        else {
            HookCrashers::LogError("[CustomSave][WriteStorage] Unexpected number of parameters: " + std::to_string(paramCount));
        }

        saveManager.commitChangesToDisk();
        ret.SetBool(success);
    }

    int GetCharacterGroupByState(int characterId, int numBaseChars, int numWorkshopChars) {
        if (characterId < numBaseChars) return 0;
        int workshopIndex = characterId - numBaseChars;
        return (workshopIndex / numWorkshopChars) + 1;
    }

    void IsCharacterUnlockedForPlayerOverride(void* thisPtr, int swfContext, uint32_t functionIdRaw, int paramCount, HC_SWFArgument** swfArgs, HC_SWFReturn* swfReturn, uint32_t callbackPtr) {
        HookCrashers::SWF::ArgsReader args(paramCount, swfArgs);
        HookCrashers::SWF::ReturnValue ret(swfReturn);

        if (paramCount != 2) {
            ret.SetBool(false);
            return;
        }

        int playerIndex = args.GetInt(0);
        int characterId = args.GetInt(1);

        if (playerIndex < 0 || playerIndex >= 4 || characterId < 0) {
            ret.SetBool(false);
            return;
        }

        if (characterId < CustomSaveManager::NUM_BASE_CHARACTERS) {
            CustomSaveManager& saveManager = CustomSaveManager::getInstance();
            const auto& entry = saveManager.getCharacterEntry(static_cast<size_t>(characterId));
            bool isUnlocked = entry.data.unlocked == 0x80;
            ret.SetBool(isUnlocked);
            return;
        }

        if (HookCrashers::IsFeatureEnabled(0x120) == 0) {
            ret.SetBool(false);
            return;
        }

        int workshopMaxIndex = CustomSaveManager::NUM_WORKSHOP_CHARACTERS * 4 - 1;
        if ((characterId - CustomSaveManager::NUM_BASE_CHARACTERS) > workshopMaxIndex) {
            ret.SetBool(false);
            return;
        }

        void* currentPlayerObject = HookCrashers::GetPlayerObject(playerIndex);
        if (!currentPlayerObject) {
            ret.SetBool(false);
            return;
        }

        char playerState = HookCrashers::GetPlayerState(currentPlayerObject);
        int requiredGroup = GetCharacterGroupByState(characterId, CustomSaveManager::NUM_BASE_CHARACTERS, CustomSaveManager::NUM_WORKSHOP_CHARACTERS);

        if (playerState != requiredGroup) {
            ret.SetBool(false);
            return;
        }

        char activeState = HookCrashers::GetPlayerActiveState(currentPlayerObject);
        int adjustedCharacterId = characterId;

        if (activeState == 0) {
            for (int otherPlayerIndex = 0; otherPlayerIndex < 4; ++otherPlayerIndex) {
                if (playerIndex == otherPlayerIndex) continue;
                void* otherPlayerObject = HookCrashers::GetPlayerObject(otherPlayerIndex);
                if (!otherPlayerObject) continue;

                if (HookCrashers::GetPlayerPosition(currentPlayerObject) == HookCrashers::GetPlayerPosition(otherPlayerObject)) {
                    char otherPlayerState = HookCrashers::GetPlayerState(otherPlayerObject);
                    if (otherPlayerState != playerState) {
                        int teamDifference = otherPlayerState - playerState;
                        int offsetPerTeam = CustomSaveManager::NUM_WORKSHOP_CHARACTERS;
                        int finalRelativeId = (characterId - CustomSaveManager::NUM_BASE_CHARACTERS) + (teamDifference * offsetPerTeam);
                        adjustedCharacterId = CustomSaveManager::NUM_BASE_CHARACTERS + finalRelativeId;
                    }
                    break;
                }
            }
        }

        bool finalCheckPassed = false;
        if (HookCrashers::Native::CallNative<char>(Natives::IsCharacterAvailableInGameMode, adjustedCharacterId) != 0) {
            finalCheckPassed = true;
        }
        else if (HookCrashers::IsOnlineMode()) {
            if (HookCrashers::Native::CallNative<int>(Natives::GetSpecialCharacterIdForTeam, playerState) == adjustedCharacterId) {
                finalCheckPassed = true;
            }
        }

        ret.SetBool(finalCheckPassed);
    }

    void LobbyTrySelectCharOverride(void* thisPtr, int swfContext, uint32_t functionIdRaw, int paramCount, HC_SWFArgument** swfArgs, HC_SWFReturn* swfReturn, uint32_t callbackPtr) {
        HookCrashers::SWF::ArgsReader args(paramCount, swfArgs);
        HookCrashers::SWF::ReturnValue ret(swfReturn);

        int resultState = 0;

        if (paramCount < 2) {
            HookCrashers::LogWarn("[LobbyTrySelectChar] Called with invalid parameter count: " + std::to_string(paramCount));
            ret.SetInt(resultState);
            return;
        }

        int playerIndex = args.GetInt(0);
        int characterId = args.GetInt(1);
        HookCrashers::LogInfo("[LobbyTrySelectChar] Attempting to select Character ID " + std::to_string(characterId) + " for Player " + std::to_string(playerIndex));

        if (HookCrashers::IsFeatureEnabled(0x120) != 0 && HookCrashers::GetGameManagerPtr() && *HookCrashers::GetGameManagerPtr() && HookCrashers::IsOnlineMode()) {
            if (HookCrashers::Native::CallNative<char>(Natives::IsCharacterAvailableInGameMode, characterId) == 0) {
                if (characterId >= CustomSaveManager::NUM_BASE_CHARACTERS) {
                    uint16_t onlineId = HookCrashers::Native::CallNative<uint16_t>(Natives::GetPlayerOnlineId, playerIndex);
                    HookCrashers::Native::CallNative<void>(Natives::UpdatePlayerIconState, 1, onlineId, 0x200);
                    resultState = 9;
                }
            }
            else {
                if (HookCrashers::Native::CallNative<char>(Natives::IsCharDLC, characterId) == 0) {
                    if (characterId < (CustomSaveManager::NUM_BASE_CHARACTERS + 1)) {
                        void* playerObj = HookCrashers::GetPlayerObject(playerIndex);
                        bool isLocalMp = (HookCrashers::Native::CallNative<char>(Natives::IsLocalMultiplayer) != 0);
                        char activeState = playerObj ? HookCrashers::GetPlayerActiveState(playerObj) : 1;
                        if (!isLocalMp || !playerObj || (activeState != 0 && HookCrashers::Native::CallNative<char>(Natives::IsBaseCharAvailableInMode, characterId) == 0)) {
                            resultState = 0;
                        }
                        else {
                            HookCrashers::Native::CallNative<void>(Natives::Native_Func_AE4AC0);
                            resultState = 2;
                        }
                    }
                    else {
                        char isAvail = HookCrashers::Native::CallNative<char>(Natives::IsWorkshopCharAvailableInMode, characterId);
                        resultState = (isAvail == 0) * 2 + 1;
                    }
                }
            }
        }

        HookCrashers::LogInfo("[LobbyTrySelectChar] Result state for character " + std::to_string(characterId) + ": " + std::to_string(resultState));
        ret.SetInt(resultState);
    }

    void LobbySkinCharAvailOverride(void* thisPtr, int swfContext, uint32_t functionIdRaw, int paramCount, HC_SWFArgument** swfArgs, HC_SWFReturn* swfReturn, uint32_t callbackPtr) {
        HookCrashers::SWF::ArgsReader args(paramCount, swfArgs);
        HookCrashers::SWF::ReturnValue ret(swfReturn);

        bool isAvailable = true;

        if (paramCount != 2) {
            HookCrashers::LogWarn("[LobbySkinAvail] Called with invalid parameter count: " + std::to_string(paramCount));
            ret.SetBool(false);
            return;
        }

        int playerIndex = args.GetInt(0);
        int characterId = args.GetInt(1);
        HookCrashers::LogInfo("[LobbySkinAvail] Checking availability for Player " + std::to_string(playerIndex) + ", Character ID " + std::to_string(characterId));

        if (HookCrashers::IsFeatureEnabled(0x120) != 0 && HookCrashers::GetGameManagerPtr() && *HookCrashers::GetGameManagerPtr()) {
            bool isOnline = HookCrashers::IsOnlineMode();
            bool isLobbyActive = isOnline ? true : (HookCrashers::Native::CallNative<char>(Natives::IsInCharSelect) != 0);

            if (HookCrashers::Native::CallNative<char>(Natives::IsCharacterAvailableInGameMode, characterId) != 0) {
                if (!isLobbyActive || !isOnline || HookCrashers::Native::CallNative<char>(Natives::IsCharDLC, characterId) != 0) {
                    if (characterId >= CustomSaveManager::NUM_BASE_CHARACTERS) {
                        void* currentPlayer = HookCrashers::GetPlayerObject(playerIndex);
                        if (!currentPlayer) { ret.SetBool(false); return; }

                        for (int otherIdx = 0; otherIdx < 4; ++otherIdx) {
                            if (playerIndex == otherIdx) continue;
                            void* otherPlayer = HookCrashers::GetPlayerObject(otherIdx);
                            if (!otherPlayer || otherPlayer == currentPlayer) continue;

                            if (HookCrashers::GetPlayerPosition(currentPlayer) == HookCrashers::GetPlayerPosition(otherPlayer) && isLobbyActive) {
                                int otherCharId = HookCrashers::GetPlayerSelectedCharacterType(otherPlayer);

                                if (otherCharId >= CustomSaveManager::NUM_BASE_CHARACTERS) {
                                    int ourSlot = (characterId - CustomSaveManager::NUM_BASE_CHARACTERS) % CustomSaveManager::NUM_WORKSHOP_CHARACTERS;
                                    int otherSlot = (otherCharId - CustomSaveManager::NUM_BASE_CHARACTERS) % CustomSaveManager::NUM_WORKSHOP_CHARACTERS;
                                    if (ourSlot == otherSlot) {
                                        isAvailable = false;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                else {
                    if (characterId < (CustomSaveManager::NUM_BASE_CHARACTERS + 1)) {
                        void* currentPlayer = HookCrashers::GetPlayerObject(playerIndex);
                        bool isLocalMp = (HookCrashers::Native::CallNative<char>(Natives::IsLocalMultiplayer) != 0);
                        char activeState = currentPlayer ? HookCrashers::GetPlayerActiveState(currentPlayer) : 1;
                        isAvailable = !isLocalMp || !currentPlayer || activeState != 0;
                    }
                    else {
                        isAvailable = false;
                    }
                }
            }
        }

        HookCrashers::LogInfo("[LobbySkinAvail] Final result for character " + std::to_string(characterId) + ": " + (isAvailable ? "AVAILABLE" : "UNAVAILABLE"));
        ret.SetBool(isAvailable);
    }

    void LobbyTryReadySkinsOverride(void* thisPtr, int swfContext, uint32_t functionIdRaw, int paramCount, HC_SWFArgument** swfArgs, HC_SWFReturn* swfReturn, uint32_t callbackPtr) {
        HookCrashers::SWF::ReturnValue ret(swfReturn);

        int result = 0;
        HookCrashers::LogInfo("[LobbyTryReadySkins] Called.");

        if (HookCrashers::IsFeatureEnabled(0x120) != 0) {
            if (HookCrashers::IsOnlineMode()) {
                // Chiamata omessa per ora come nell'originale
            }

            HookCrashers::Native::CallNative<void>(Natives::Native_Func_AE4AC0);
            result = HookCrashers::Native::CallNative<int>(Natives::GetTeamId);

            if (result != 0 && HookCrashers::IsOnlineMode() && HookCrashers::Native::CallNative<char>(Natives::IsTeamReady) != 0) {
                HookCrashers::Native::CallNative<void>(Natives::FinalizeTeamSelection);
                result = -1;
            }
        }

        HookCrashers::LogInfo("[LobbyTryReadySkins] Result: " + std::to_string(result));
        ret.SetInt(result);
    }


    void RegisterStorageOverrides() {
        HookCrashers::LogInfo("[CustomSave] Registering storage overrides...");

        if (HookCrashers::RegisterOverride(SWF_ID_READ_STORAGE, ReadStorageOverride)) {
            HookCrashers::LogInfo("[CustomSave] ReadStorage override registered (ID: 0x3F).");
        }
        else {
            HookCrashers::LogError("[CustomSave] Failed to register ReadStorage override!");
        }

        if (HookCrashers::RegisterOverride(SWF_ID_WRITE_STORAGE, WriteStorageOverride)) {
            HookCrashers::LogInfo("[CustomSave] WriteStorage override registered (ID: 0x40).");
        }
        else {
            HookCrashers::LogError("[CustomSave] Failed to register WriteStorage override!");
        }

        /*if (HookCrashers::RegisterOverride(SWF_ID_IS_CHARACTER_UNLOCKED_FOR_PLAYER, IsCharacterUnlockedForPlayerOverride)) {
            HookCrashers::LogInfo("[CustomSave] IsCharacterUnlockedForPlayer override registered (ID: 0xC4).");
        }
        else {
            HookCrashers::LogError("[CustomSave] Failed to register IsCharacterUnlockedForPlayer override!");
        }

        if (HookCrashers::RegisterOverride(SWF_ID_LOBBY_TRY_SELECT_CHAR, LobbyTrySelectCharOverride)) {
            HookCrashers::LogInfo("[Overrides] Successfully registered LobbyTrySelectChar override (0xE5).");
        }
        else {
            HookCrashers::LogError("[Overrides] FAILED to register LobbyTrySelectChar override (0xE5)!");
        }

        if (HookCrashers::RegisterOverride(SWF_ID_LOBBY_SKIN_CHAR_AVAIL, LobbySkinCharAvailOverride)) {
            HookCrashers::LogInfo("[CustomSave] LobbySkinCharAvail override registered (ID: 0xE6).");
        }
        else {
            HookCrashers::LogError("[CustomSave] Failed to register LobbySkinCharAvail override!");
        }

        if (HookCrashers::RegisterOverride(SWF_ID_LOBBY_TRY_READY_SKINS, LobbyTryReadySkinsOverride)) {
            HookCrashers::LogInfo("[Overrides] Successfully registered LobbyTryReadySkins override (0xE7).");
        }
        else {
            HookCrashers::LogError("[Overrides] FAILED to register LobbyTryReadySkins override (0xE7)!");
        }*/

        HookCrashers::LogInfo("[CustomSave] Storage override registration complete.");
    }
}