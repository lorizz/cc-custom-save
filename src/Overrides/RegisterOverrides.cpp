// RegisterOverrides.cpp
#include "RegisterOverrides.h"
#include "../Core/CustomSaveManager.h"
#include <HookCrashersAPI.h>

std::unordered_map<uint16_t, HookCrashers::API::InternalCustomCallback> HookCrashers::API::Client::s_customCallbacks;
std::unordered_map<uint16_t, HookCrashers::API::InternalOverrideCallback> HookCrashers::API::Client::s_overrideCallbacks;

// SWF Function IDs
#define SWF_ID_READ_STORAGE     0x3F 
#define SWF_ID_WRITE_STORAGE    0x40
#define SWF_ID_IS_CHARACTER_UNLOCKED_FOR_PLAYER 0xC4
#define SWF_ID_LOBBY_TRY_SELECT_CHAR 0xE5
#define SWF_ID_LOBBY_SKIN_CHAR_AVAIL 0xE6
#define SWF_ID_LOBBY_TRY_READY_SKINS 0xE7

namespace Natives = HookCrashers::Native::Natives;

using HookCrashers::API::Client;
using HookCrashers::API::ReturnHelper;
using HookCrashers::SWF::Data::SWFArgument;
using HookCrashers::SWF::Data::SWFReturn;
using HookCrashers::Native::CallNative;

// Static offsets for sequential reads/writes
static size_t s_current_read_offset = 0;
static size_t s_current_write_offset = 0;

namespace CustomSave {
    // Helper to get an integer argument safely
    int GetIntArg(SWFArgument** args, int index, int paramCount) {
        if (index < paramCount && args[index] && args[index]->type == SWFArgument::Type::Integer) {
            return args[index]->value.intValue;
        }
        return 0;
    }

    // ReadStorage override (ID 0x3F)
    // Parameters: (playerPort) or (playerPort, offset)
    // Reads a byte from the game's conceptual save data.
    void ReadStorageOverride(void* thisPtr, int swfContext, uint32_t functionIdRaw, int paramCount, SWFArgument** swfArgs, SWFReturn* swfReturn, uint32_t callbackPtr) {
        CustomSave::CustomSaveManager& saveManager = CustomSave::CustomSaveManager::getInstance();
        uint8_t byte_value = 0;
        // playerPort is swfArgs[0], always present for these calls
        int playerPort = GetIntArg(swfArgs, 0, paramCount);

        if (paramCount == 2) { // (playerPort, offset)
            int new_offset = GetIntArg(swfArgs, 1, paramCount);
            s_current_read_offset = new_offset; // Set new offset

            byte_value = saveManager.readByteFromGameBuffer(s_current_read_offset);
            s_current_read_offset++; // Advance for next sequential read

        }
        else if (paramCount == 1) { // (playerPort) - sequential read
            byte_value = saveManager.readByteFromGameBuffer(s_current_read_offset);
            s_current_read_offset++; // Advance for next sequential read
        }
        else {
            Client::LogError("[CustomSave][ReadStorage] Unexpected number of parameters: " + std::to_string(paramCount));
        }

        ReturnHelper::SetInt(swfReturn, byte_value);
    }

    // WriteStorage override (ID 0x40)
    // Parameters: (playerPort, value) or (playerPort, offset, value)
    // Writes a byte to the game's conceptual save data.
    void WriteStorageOverride(void* thisPtr, int swfContext, uint32_t functionIdRaw, int paramCount, SWFArgument** swfArgs, SWFReturn* swfReturn, uint32_t callbackPtr) {
        CustomSave::CustomSaveManager& saveManager = CustomSave::CustomSaveManager::getInstance();
        bool success = false;

        // playerPort is swfArgs[0], always present for these calls
        int playerPort = GetIntArg(swfArgs, 0, paramCount);

        if (paramCount == 3) { // (playerPort, offset, value)
            int new_offset = GetIntArg(swfArgs, 1, paramCount);
            int value = GetIntArg(swfArgs, 2, paramCount);
            s_current_write_offset = new_offset; // Set new offset

            saveManager.writeByteToGameBuffer(s_current_write_offset, static_cast<uint8_t>(value));
            s_current_write_offset++; // Advance for next sequential write
            success = true;

        }
        else if (paramCount == 2) { // (playerPort, value) - sequential write
            int value = GetIntArg(swfArgs, 1, paramCount);

            saveManager.writeByteToGameBuffer(s_current_write_offset, static_cast<uint8_t>(value));
            s_current_write_offset++; // Advance for next sequential write
            success = true;
        }
        else {
            Client::LogError("[CustomSave][WriteStorage] Unexpected number of parameters: " + std::to_string(paramCount));
        }

        // Commit changes to disk. This might be inefficient if writes are frequent.
        // A more robust solution would be to hook into the game's actual "save game" event.
        saveManager.commitChangesToDisk();

        ReturnHelper::SetBool(swfReturn, success);
    }

    int GetCharacterGroupByState(int characterId, int numBaseChars, int numWorkshopChars) {
        if (characterId < numBaseChars) return 0;
        int workshopIndex = characterId - numBaseChars;
        return (workshopIndex / numWorkshopChars) + 1;
    }

    void IsCharacterUnlockedForPlayerOverride(void* thisPtr, int swfContext, uint32_t functionIdRaw, int paramCount, SWFArgument** swfArgs, SWFReturn* swfReturn, uint32_t callbackPtr) {
        // --- Setup e Validazione Iniziale ---
        if (paramCount != 2) {
            ReturnHelper::SetBool(swfReturn, false);
            return;
        }

        int playerIndex = GetIntArg(swfArgs, 0, paramCount);
        int characterId = GetIntArg(swfArgs, 1, paramCount);


        if (playerIndex < 0 || playerIndex >= 4 || characterId < 0) {
            ReturnHelper::SetBool(swfReturn, false);
            return;
        }

        // --- Gestione Personaggi Base ---
        if (characterId < CustomSave::CustomSaveManager::NUM_BASE_CHARACTERS) {
            CustomSave::CustomSaveManager& saveManager = CustomSave::CustomSaveManager::getInstance();
            const auto& entry = saveManager.getCharacterEntry(static_cast<size_t>(characterId));
            bool isUnlocked = entry.data.unlocked == 0x80;
            ReturnHelper::SetBool(swfReturn, isUnlocked);
            return;
        }

        // --- Controlli Workshop ---
        uint32_t workshopEnabled = Client::IsFeatureEnabled(0x120);
        int workshopMaxIndex = CustomSaveManager::NUM_WORKSHOP_CHARACTERS * 4 - 1;

        if (workshopEnabled == 0) {
            ReturnHelper::SetBool(swfReturn, false);
            return;
        }

        if ((characterId - CustomSaveManager::NUM_BASE_CHARACTERS) > workshopMaxIndex) {
            ReturnHelper::SetBool(swfReturn, false);
            return;
        }

        void* currentPlayerObject = Client::GetPlayerObject(playerIndex);
        if (!currentPlayerObject) {
            ReturnHelper::SetBool(swfReturn, false);
            return;
        }

        char playerState = Client::GetPlayerState(currentPlayerObject);

        // --- 1. VERIFICA APPARTENENZA A GRUPPO/TEAM ---
        int requiredGroup = GetCharacterGroupByState(characterId, CustomSaveManager::NUM_BASE_CHARACTERS, CustomSaveManager::NUM_WORKSHOP_CHARACTERS);

        if (playerState != requiredGroup) {
            ReturnHelper::SetBool(swfReturn, false);
            return;
        }

        // --- 2. VERIFICA CONFLITTI NEL TEAM ---
        char activeState = Client::GetPlayerActiveState(currentPlayerObject);

        int adjustedCharacterId = characterId;

        if (activeState == 0) { // Giocatore in menu di selezione
            for (int otherPlayerIndex = 0; otherPlayerIndex < 4; ++otherPlayerIndex) {
                if (playerIndex == otherPlayerIndex) continue;

                void* otherPlayerObject = Client::GetPlayerObject(otherPlayerIndex);
                if (!otherPlayerObject) continue;

                uint64_t currentPlayerPos = Client::GetPlayerPosition(currentPlayerObject);
                uint64_t otherPlayerPos = Client::GetPlayerPosition(otherPlayerObject);

                if (currentPlayerPos == otherPlayerPos) { // Trovato compagno di squadra

                    char otherPlayerState = Client::GetPlayerState(otherPlayerObject);

                    if (otherPlayerState != playerState) {
                        int relativeCharId = characterId - CustomSaveManager::NUM_BASE_CHARACTERS;

                        // Questa è la traduzione di:
                        // adjustedCharacterId = (int *)((int)local_134 + (bVar1 - uVar16) * 10);
                        int teamDifference = otherPlayerState - playerState;
                        int offsetPerTeam = CustomSaveManager::NUM_WORKSHOP_CHARACTERS;
                        int finalRelativeId = relativeCharId + (teamDifference * offsetPerTeam);

                        adjustedCharacterId = CustomSaveManager::NUM_BASE_CHARACTERS + finalRelativeId;

                    }

                    // Il codice originale esce dopo aver trovato il primo compagno.
                    break;
                }
            }
        }

        // --- 3. VERIFICA FINALE SULLO STATO DEL GIOCO (SPECULATIVA) ---
        bool finalCheckPassed = false;
        bool isAvailableInMode = CallNative<char>(Natives::IsCharacterAvailableInGameMode, adjustedCharacterId) != 0;
        if (isAvailableInMode) {
            finalCheckPassed = true;
        }
        else {
            bool isInOnlineMode = Client::IsOnlineMode();
            if (isInOnlineMode) {
                int specialCharId = CallNative<int>(Natives::GetSpecialCharacterIdForTeam, playerState);
                bool specialIdMatches = (specialCharId == adjustedCharacterId);
                if (specialIdMatches) {
                    finalCheckPassed = true;
                }
            }
        }

        if (finalCheckPassed) {
            ReturnHelper::SetBool(swfReturn, true);
            return;
        }
        else {
            ReturnHelper::SetBool(swfReturn, false);
            return;
        }


        ReturnHelper::SetBool(swfReturn, true);
    }

    void LobbyTrySelectCharOverride(void* thisPtr, int swfContext, uint32_t functionIdRaw, int paramCount, SWFArgument** swfArgs, SWFReturn* swfReturn, uint32_t callbackPtr) {
        int resultState = 0; // Default: 0 (Success/No Change)

        if (paramCount != 2) { // Il secondo parametro (frame del ritratto) è ignorato
            Client::LogWarn("[LobbyTrySelectChar] Called with invalid parameter count: " + std::to_string(paramCount));
            ReturnHelper::SetInt(swfReturn, resultState); return;
        }

        int playerIndex = GetIntArg(swfArgs, 0, paramCount);
        int characterId = GetIntArg(swfArgs, 1, paramCount);
        Client::LogInfo("[LobbyTrySelectChar] Attempting to select Character ID " + std::to_string(characterId) + " for Player " + std::to_string(playerIndex));

        if (Client::IsFeatureEnabled(0x120) != 0 && Client::GetGameManagerPtr() && *Client::GetGameManagerPtr() && Client::IsOnlineMode()) {
            if (CallNative<char>(Natives::IsCharacterAvailableInGameMode, characterId) == 0) {
                // if (0x20 < iVar17) -> IL FIX
                if (characterId >= CustomSaveManager::NUM_BASE_CHARACTERS) {
                    uint16_t onlineId = CallNative<uint16_t>(Natives::GetPlayerOnlineId, playerIndex);
                    CallNative<void>(Natives::UpdatePlayerIconState, 1, onlineId, 0x200);
                    resultState = 9; // Codice di errore/stato per "personaggio non disponibile"
                }
            }
            else {
                if (CallNative<char>(Natives::IsCharDLC, characterId) == 0) {
                    // if (iVar17 < 0x21) -> IL FIX
                    if (characterId < (CustomSaveManager::NUM_BASE_CHARACTERS + 1)) {
                        void* playerObj = Client::GetPlayerObject(playerIndex);
                        bool isLocalMp = (CallNative<char>(Natives::IsLocalMultiplayer) != 0);
                        char activeState = playerObj ? Client::GetPlayerActiveState(playerObj) : 1;

                        if (!isLocalMp || !playerObj || (activeState != 0 && CallNative<char>(Natives::IsBaseCharAvailableInMode, characterId) == 0)) {
                            resultState = 0; // Fallisce silenziosamente
                        }
                        else {
                            CallNative<void>(Natives::Native_Func_AE4AC0);
                            resultState = 2; // Codice per "selezione ok, ma richiede conferma"
                        }
                    }
                    else {
                        char isAvail = CallNative<char>(Natives::IsWorkshopCharAvailableInMode, characterId);
                        resultState = (isAvail == 0) * 2 + 1; // Converte (0 -> 1, 1 -> 3)
                    }
                }
            }
        }

        Client::LogInfo("[LobbyTrySelectChar] Result state for character " + std::to_string(characterId) + ": " + std::to_string(resultState));
        ReturnHelper::SetInt(swfReturn, resultState);
    }

    void LobbySkinCharAvailOverride(void* thisPtr, int swfContext, uint32_t functionIdRaw, int paramCount, SWFArgument** swfArgs, SWFReturn* swfReturn, uint32_t callbackPtr) {
        bool isAvailable = true; // Default

        if (paramCount != 2) {
            Client::LogWarn("[LobbySkinAvail] Called with invalid parameter count: " + std::to_string(paramCount));
            ReturnHelper::SetBool(swfReturn, false); return;
        }

        int playerIndex = GetIntArg(swfArgs, 0, paramCount);
        int characterId = GetIntArg(swfArgs, 1, paramCount);
        Client::LogInfo("[LobbySkinAvail] Checking availability for Player " + std::to_string(playerIndex) + ", Character ID " + std::to_string(characterId));

        if (Client::IsFeatureEnabled(0x120) != 0 && Client::GetGameManagerPtr() && *Client::GetGameManagerPtr()) {
            bool isOnline = Client::IsOnlineMode();
            bool isLobbyActive = isOnline ? true : (CallNative<char>(Natives::IsInCharSelect) != 0);

            if (CallNative<char>(Natives::IsCharacterAvailableInGameMode, characterId) != 0) {
                if (!isLobbyActive || !isOnline || CallNative<char>(Natives::IsCharDLC, characterId) != 0) {
                    // --- WORKSHOP CHARACTER LOGIC ---
                    if (characterId >= CustomSaveManager::NUM_BASE_CHARACTERS) { // <-- IL FIX
                        void* currentPlayer = Client::GetPlayerObject(playerIndex);
                        if (!currentPlayer) { /* ... error log ... */ ReturnHelper::SetBool(swfReturn, false); return; }

                        for (int otherIdx = 0; otherIdx < 4; ++otherIdx) {
                            if (playerIndex == otherIdx) continue;
                            void* otherPlayer = Client::GetPlayerObject(otherIdx);
                            if (!otherPlayer || otherPlayer == currentPlayer) continue;

                            if (Client::GetPlayerPosition(currentPlayer) == Client::GetPlayerPosition(otherPlayer) && isLobbyActive) {
                                int otherCharId = -1;
                                if (isOnline) {
                                    otherCharId = Client::GetPlayerSelectedCharacterType(otherPlayer);
                                }
                                else {
                                    // Logica per offline/char select menu: il gioco usa un altro metodo per ottenere il char.
                                    // Per semplicità, e dato che il conflitto principale è online, possiamo assumere
                                    // che GetPlayerSelectedCharacterType sia sufficiente o che la logica offline sia meno restrittiva.
                                    // La logica offline usa FUN_00a98040 e FUN_00b7a470, che sono più complessi da replicare.
                                    // Focalizziamoci sul fix principale.
                                    // TODO remember to fix
                                }

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
                    // --- BASE CHARACTER LOGIC ---
                    if (characterId < (CustomSaveManager::NUM_BASE_CHARACTERS + 1)) { // <-- IL FIX (era < 0x21)
                        void* currentPlayer = Client::GetPlayerObject(playerIndex);
                        bool isLocalMp = (CallNative<char>(Natives::IsLocalMultiplayer) != 0);
                        char activeState = currentPlayer ? Client::GetPlayerActiveState(currentPlayer) : 1;
                        if (!isLocalMp || !currentPlayer || activeState != 0) {
                            isAvailable = true;
                        }
                        else {
                            isAvailable = false;
                        }
                    }
                    else {
                        isAvailable = false;
                    }
                }
            }
        }

        Client::LogInfo("[LobbySkinAvail] Final result for character " + std::to_string(characterId) + ": " + std::string(isAvailable ? "AVAILABLE" : "UNAVAILABLE"));
        ReturnHelper::SetBool(swfReturn, isAvailable);
    }

    void LobbyTryReadySkinsOverride(void* thisPtr, int swfContext, uint32_t functionIdRaw, int paramCount, SWFArgument** swfArgs, SWFReturn* swfReturn, uint32_t callbackPtr) {
        int result = 0; // Default
        Client::LogInfo("[LobbyTryReadySkins] Called.");

        if (Client::IsFeatureEnabled(0x120) != 0) {
            if (Client::IsOnlineMode()) {
                char localMp = CallNative<char>(Natives::IsLocalMultiplayer);
                // Il disassemblato passa due parametri extra a FUN_00aedda0 che non sono
                // chiaramente definiti; potrebbero essere in ECX e EDX.
                // Per ora, li omettiamo in quanto la logica principale segue.
                // CallNative<void>(Native::Natives::Native_Func_AEDDA0, localMp, ...);
            }

            CallNative<void>(Natives::Native_Func_AE4AC0);
            result = CallNative<int>(Natives::GetTeamId);

            if (result != 0 && Client::IsOnlineMode() && CallNative<char>(Natives::IsTeamReady) != 0) {
                CallNative<void>(Natives::FinalizeTeamSelection);
                result = -1;
            }
        }

        Client::LogInfo("[LobbyTryReadySkins] Result: " + std::to_string(result));
        ReturnHelper::SetInt(swfReturn, result);
    }


    void RegisterStorageOverrides() {
        Client::LogInfo("[CustomSave] Registering storage overrides...");

        if (!Client::RegisterOverride(SWF_ID_READ_STORAGE, ReadStorageOverride)) {
            Client::LogError("[CustomSave] Failed to register ReadStorage override!");
        }
        else {
            Client::LogInfo("[CustomSave] ReadStorage override registered (ID: 0x3F).");
        }

        if (!Client::RegisterOverride(SWF_ID_WRITE_STORAGE, WriteStorageOverride)) {
            Client::LogError("[CustomSave] Failed to register WriteStorage override!");
        }
        else {
            Client::LogInfo("[CustomSave] WriteStorage override registered (ID: 0x40).");
        }

        if (!Client::RegisterOverride(SWF_ID_IS_CHARACTER_UNLOCKED_FOR_PLAYER, IsCharacterUnlockedForPlayerOverride)) {
            Client::LogError("[CustomSave] Failed to register IsCharacterUnlockedForPlayer override!");
        }
        else {
            Client::LogInfo("[CustomSave] IsCharacterUnlockedForPlayer override registered (ID: 0xC4).");
        }

        if (!Client::RegisterOverride(SWF_ID_LOBBY_TRY_SELECT_CHAR, LobbyTrySelectCharOverride)) {
            Client::LogError("[Overrides] FAILED to register LobbyTrySelectChar override (0xE5)!");
        }
        else {
            Client::LogInfo("[Overrides] Successfully registered LobbyTrySelectChar override (0xE5).");
        }

        if (!Client::RegisterOverride(SWF_ID_LOBBY_SKIN_CHAR_AVAIL, LobbySkinCharAvailOverride)) {
            Client::LogError("[CustomSave] Failed to register LobbySkinCharAvail override!");
        }
        else {
            Client::LogInfo("[CustomSave] LobbySkinCharAvail override registered (ID: 0xE6).");
        }

        if (!Client::RegisterOverride(SWF_ID_LOBBY_TRY_READY_SKINS, LobbyTryReadySkinsOverride)) {
            Client::LogError("[Overrides] FAILED to register LobbyTryReadySkins override (0xE7)!");
        }
        else {
            Client::LogInfo("[Overrides] Successfully registered LobbyTryReadySkins override (0xE7).");
        }
        Client::LogInfo("[CustomSave] Storage override registration complete.");
    }
}