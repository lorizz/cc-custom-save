#include <fstream>
#include <vector>
#include <json.hpp>
#include "storage_hooks.h"
#include "default_character_data.h"

using json = nlohmann::json;

const std::string JSON_FILENAME = "scripts/saves/game_data.json";

// Function to convert GlobalUnlocks to JSON
json globalUnlocksToJson(const GlobalUnlocks& unlocks) {
    return json{
        {"unknownByte", unlocks.unknownByte},
        {"voiceVolume", unlocks.voiceVolume},
        {"musicVolume", unlocks.musicVolume},
        {"sfxVolume", unlocks.sfxVolume},
        {"unknownArray1", unlocks.unknownArray1},
        {"arenas", unlocks.arenas},
        {"animals", unlocks.animals},
        {"itemUnlocks", unlocks.itemUnlocks},
        {"relics", unlocks.relics},
        {"weapons", unlocks.weapons},
        {"dlcWeapons", unlocks.dlcWeapons},
        {"unknownArray2", unlocks.unknownArray2},
        {"backOffBarbarianTime", unlocks.backOffBarbarianTime},
        {"backOffBarbarianBestChar", unlocks.backOffBarbarianBestChar},
        {"unknownArray3", unlocks.unknownArray3}
    };
}

// Function to convert JSON to GlobalUnlocks
GlobalUnlocks jsonToGlobalUnlocks(const json& j) {
    GlobalUnlocks unlocks;
    unlocks.unknownByte = j["unknownByte"];
    unlocks.voiceVolume = j["voiceVolume"];
    unlocks.musicVolume = j["musicVolume"];
    unlocks.sfxVolume = j["sfxVolume"];
    unlocks.unknownArray1 = j["unknownArray1"].get<std::array<uint8_t, 4>>();
    unlocks.arenas = j["arenas"];
    unlocks.animals = j["animals"].get<std::array<uint8_t, 4>>();
    unlocks.itemUnlocks = j["itemUnlocks"].get<std::array<uint8_t, 4>>();
    unlocks.relics = j["relics"];
    unlocks.weapons = j["weapons"].get<std::array<uint8_t, 12>>();
    unlocks.dlcWeapons = j["dlcWeapons"].get<std::array<uint8_t, 8>>();
    unlocks.unknownArray2 = j["unknownArray2"].get<std::array<uint8_t, 10>>();
    unlocks.backOffBarbarianTime = j["backOffBarbarianTime"].get<std::array<uint8_t, 4>>();
    unlocks.backOffBarbarianBestChar = j["backOffBarbarianBestChar"];
    unlocks.unknownArray3 = j["unknownArray3"].get<std::array<uint8_t, 11>>();
    return unlocks;
}

// Function to convert CharacterData to JSON
json characterDataToJson(const CharacterData& data) {
    return json{
        {"unlockStatus", data.unlockStatus},
        {"level", data.level},
        {"xp", data.xp},
        {"weapon", data.weapon},
        {"pet", data.pet},
        {"stats", data.stats},
        {"normalLevelUnlocks", data.normalLevelUnlocks},
        {"consumableItems", data.consumableItems},
        {"nonConsumableItems", data.nonConsumableItems},
        {"money", data.money},
        {"insaneModeStoreUnlock", data.insaneModeStoreUnlock},
        {"insaneLevelUnlocks", data.insaneLevelUnlocks},
        {"skullStatus", data.skullStatus},
        {"dugUpItems", data.dugUpItems},
        {"princessKisses", data.princessKisses},
        {"arenaWins", data.arenaWins},
        {"unknown", data.unknown}
    };
}

// Function to convert JSON to CharacterData
CharacterData jsonToCharacterData(const json& j) {
    CharacterData data;
    data.unlockStatus = j["unlockStatus"];
    data.level = j["level"];
    data.xp = j["xp"].get<std::array<uint8_t, 4>>();
    data.weapon = j["weapon"];
    data.pet = j["pet"];
    data.stats = j["stats"].get<std::array<uint8_t, 4>>();
    data.normalLevelUnlocks = j["normalLevelUnlocks"].get<std::array<uint8_t, 3>>();
    data.consumableItems = j["consumableItems"].get<std::array<uint8_t, 3>>();
    data.nonConsumableItems = j["nonConsumableItems"];
    data.money = j["money"].get<std::array<uint8_t, 4>>();
    data.insaneModeStoreUnlock = j["insaneModeStoreUnlock"];
    data.insaneLevelUnlocks = j["insaneLevelUnlocks"].get<std::array<uint8_t, 3>>();
    data.skullStatus = j["skullStatus"];
    data.dugUpItems = j["dugUpItems"];
    data.princessKisses = j["princessKisses"];
    data.arenaWins = j["arenaWins"].get<std::array<uint8_t, 2>>();
    data.unknown = j["unknown"].get<std::array<uint8_t, 16>>();
    return data;
}

GlobalUnlocks InitializeGlobalUnlocks() {
    GlobalUnlocks initGlobalUnlocks;

    initGlobalUnlocks.unknownByte = 0x00;
    initGlobalUnlocks.voiceVolume = 0x05;
    initGlobalUnlocks.musicVolume = 0x05;
    initGlobalUnlocks.sfxVolume = 0x05;
    initGlobalUnlocks.unknownArray1 = { 0x00, 0x00, 0x00, 0x00 };
    initGlobalUnlocks.arenas = 0x00;
    initGlobalUnlocks.animals = { 0x00, 0x00, 0x00, 0x00 };
    initGlobalUnlocks.itemUnlocks = { 0x00, 0x00, 0x00, 0x00 };
    initGlobalUnlocks.relics = 0x00;
    initGlobalUnlocks.weapons = { 0x20, 0x00, 0x00, 0x08, 0x00, 0x02, 0x00, 0x05, 0x02, 0xF6, 0xAF, 0x00 };
    initGlobalUnlocks.dlcWeapons = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    initGlobalUnlocks.unknownArray2 = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    initGlobalUnlocks.backOffBarbarianTime = { 0x00, 0x00, 0x00, 0x00 };
    initGlobalUnlocks.backOffBarbarianBestChar = 0x00;
    initGlobalUnlocks.unknownArray3 = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    return initGlobalUnlocks;
}

void InitializeJsonFile() {
    Logger& l = Logger::Instance();

    // Check if the file already exists
    std::ifstream fileCheck(JSON_FILENAME);
    if (fileCheck.is_open()) {
        fileCheck.close();
        l.Get()->info("JSON file already exists: {}", JSON_FILENAME);
        l.Get()->flush();
        return;
    }

    l.Get()->info("JSON file not found, creating with default data");
    l.Get()->flush();

    // Create JSON object with default global unlocks and character data
    json gameData;
    gameData["globalUnlocks"] = globalUnlocksToJson(InitializeGlobalUnlocks());

    // Create JSON array with default character data
    json characterArray = json::array();
    characterArray.push_back(characterDataToJson(greenKnight));
    characterArray.push_back(characterDataToJson(redKnight));
    characterArray.push_back(characterDataToJson(blueKnight));
    characterArray.push_back(characterDataToJson(orangeKnight));
    characterArray.push_back(characterDataToJson(grayKnight));
    characterArray.push_back(characterDataToJson(barbarian));
    characterArray.push_back(characterDataToJson(thief));
    characterArray.push_back(characterDataToJson(fencer));
    characterArray.push_back(characterDataToJson(beekeeper));
    characterArray.push_back(characterDataToJson(industrialist));
    characterArray.push_back(characterDataToJson(alien));
    characterArray.push_back(characterDataToJson(king));
    characterArray.push_back(characterDataToJson(brute));
    characterArray.push_back(characterDataToJson(snakey));
    characterArray.push_back(characterDataToJson(saracen));
    characterArray.push_back(characterDataToJson(royalGuard));
    characterArray.push_back(characterDataToJson(stoveface));
    characterArray.push_back(characterDataToJson(peasant));
    characterArray.push_back(characterDataToJson(bear));
    characterArray.push_back(characterDataToJson(necromancer));
    characterArray.push_back(characterDataToJson(conehead));
    characterArray.push_back(characterDataToJson(civilian));
    characterArray.push_back(characterDataToJson(openGrayKnight));
    characterArray.push_back(characterDataToJson(fireDemon));
    characterArray.push_back(characterDataToJson(skeleton));
    characterArray.push_back(characterDataToJson(iceskimo));
    characterArray.push_back(characterDataToJson(ninja));
    characterArray.push_back(characterDataToJson(cultist));
    characterArray.push_back(characterDataToJson(pinkKnight));
    characterArray.push_back(characterDataToJson(blacksmith));
    characterArray.push_back(characterDataToJson(hatty));

    gameData["characters"] = characterArray;

    // Write JSON to file
    std::ofstream file(JSON_FILENAME);
    if (!file.is_open()) {
        l.Get()->error("Failed to create JSON file: {}", JSON_FILENAME);
        l.Get()->flush();
        return;
    }

    file << std::setw(4) << gameData << std::endl;
    file.close();

    l.Get()->info("Created JSON file with default character data: {}", JSON_FILENAME);
    l.Get()->flush();
}

void WriteByteToJson(uint32_t offset, uint8_t byteValue) {
    Logger& l = Logger::Instance();

    // Read the entire JSON file
    std::ifstream inputFile(JSON_FILENAME);
    if (!inputFile.is_open()) {
        l.Get()->error("Failed to open JSON file for reading: {}", JSON_FILENAME);
        l.Get()->flush();
        return;
    }

    json gameData;
    inputFile >> gameData;
    inputFile.close();

    if (offset < 64) {
        // Update GlobalUnlocks
        GlobalUnlocks globalUnlocksData = jsonToGlobalUnlocks(gameData["globalUnlocks"]);
        if (offset == 0) globalUnlocksData.unknownByte = byteValue;
        else if (offset == 1) globalUnlocksData.voiceVolume = byteValue;
        else if (offset == 2) globalUnlocksData.musicVolume = byteValue;
        else if (offset == 3) globalUnlocksData.sfxVolume = byteValue;
        else if (offset >= 4 && offset <= 7) globalUnlocksData.unknownArray1[offset - 4] = byteValue;
        else if (offset == 8) globalUnlocksData.arenas = byteValue;
        else if (offset >= 9 && offset <= 12) globalUnlocksData.animals[offset - 9] = byteValue;
        else if (offset >= 13 && offset <= 16) globalUnlocksData.itemUnlocks[offset - 13] = byteValue;
        else if (offset == 17) globalUnlocksData.relics = byteValue;
        else if (offset >= 18 && offset <= 29) globalUnlocksData.weapons[offset - 18] = byteValue;
        else if (offset >= 30 && offset <= 37) globalUnlocksData.dlcWeapons[offset - 30] = byteValue;
        else if (offset >= 38 && offset <= 47) globalUnlocksData.unknownArray2[offset - 38] = byteValue;
        else if (offset >= 48 && offset <= 51) globalUnlocksData.backOffBarbarianTime[offset - 48] = byteValue;
        else if (offset == 52) globalUnlocksData.backOffBarbarianBestChar = byteValue;
        else if (offset >= 53 && offset <= 63) globalUnlocksData.unknownArray3[offset - 53] = byteValue;

        gameData["globalUnlocks"] = globalUnlocksToJson(globalUnlocksData);
    }
    else {
        // Update CharacterData
        uint32_t characterOffset = offset - 64;
        size_t characterIndex = characterOffset / 48;  // Each character occupies 48 bytes
        size_t fieldOffset = characterOffset % 48;

        if (characterIndex >= gameData["characters"].size()) {
            l.Get()->error("Invalid character index: {}", characterIndex);
            l.Get()->flush();
            return;
        }

        l.Get()->info("Writing to offset {} for character ID: {}, value: {}", fieldOffset, characterIndex, byteValue);
        l.Get()->flush();

        CharacterData charData = jsonToCharacterData(gameData["characters"][characterIndex]);

        if (fieldOffset == 0) charData.unlockStatus = byteValue;
        else if (fieldOffset == 1) charData.level = byteValue;
        else if (fieldOffset >= 2 && fieldOffset <= 5) charData.xp[fieldOffset - 2] = byteValue;
        else if (fieldOffset == 6) charData.weapon = byteValue;
        else if (fieldOffset == 7) charData.pet = byteValue;
        else if (fieldOffset >= 8 && fieldOffset <= 11) charData.stats[fieldOffset - 8] = byteValue;
        else if (fieldOffset >= 12 && fieldOffset <= 14) charData.normalLevelUnlocks[fieldOffset - 12] = byteValue;
        else if (fieldOffset >= 15 && fieldOffset <= 17) charData.consumableItems[fieldOffset - 15] = byteValue;
        else if (fieldOffset == 18) charData.nonConsumableItems = byteValue;
        else if (fieldOffset >= 19 && fieldOffset <= 22) charData.money[fieldOffset - 19] = byteValue;
        else if (fieldOffset == 23) charData.insaneModeStoreUnlock = byteValue;
        else if (fieldOffset >= 24 && fieldOffset <= 26) charData.insaneLevelUnlocks[fieldOffset - 24] = byteValue;
        else if (fieldOffset == 27) charData.skullStatus = byteValue;
        else if (fieldOffset == 28) charData.dugUpItems = byteValue;
        else if (fieldOffset == 29) charData.princessKisses = byteValue;
        else if (fieldOffset >= 30 && fieldOffset <= 31) charData.arenaWins[fieldOffset - 30] = byteValue;
        else if (fieldOffset >= 32 && fieldOffset <= 47) charData.unknown[fieldOffset - 32] = byteValue;

        gameData["characters"][characterIndex] = characterDataToJson(charData);
    }

    // Write the updated JSON back to the file
    std::ofstream outputFile(JSON_FILENAME);
    if (!outputFile.is_open()) {
        l.Get()->error("Failed to open JSON file for writing: {}", JSON_FILENAME);
        l.Get()->flush();
        return;
    }

    outputFile << std::setw(4) << gameData << std::endl;
    outputFile.close();

    l.Get()->info("Updated byte at offset {} with value 0x{:02X}", offset, byteValue);
    l.Get()->flush();
}

uint8_t ReadByteFromJson(uint32_t offset) {
    Logger& l = Logger::Instance();

    // Read the entire JSON file
    std::ifstream inputFile(JSON_FILENAME);
    if (!inputFile.is_open()) {
        l.Get()->error("Failed to open JSON file for reading: {}", JSON_FILENAME);
        l.Get()->flush();
        throw std::runtime_error("Failed to open JSON file");
    }

    json gameData;
    inputFile >> gameData;
    inputFile.close();

    uint8_t byteValue = 0;

    if (offset < 64) {
        // Read from GlobalUnlocks
        GlobalUnlocks globalUnlocksData = jsonToGlobalUnlocks(gameData["globalUnlocks"]);
        if (offset == 0) byteValue = globalUnlocksData.unknownByte;
        else if (offset == 1) byteValue = globalUnlocksData.voiceVolume;
        else if (offset == 2) byteValue = globalUnlocksData.musicVolume;
        else if (offset == 3) byteValue = globalUnlocksData.sfxVolume;
        else if (offset >= 4 && offset <= 7) byteValue = globalUnlocksData.unknownArray1[offset - 4];
        else if (offset == 8) byteValue = globalUnlocksData.arenas;
        else if (offset >= 9 && offset <= 12) byteValue = globalUnlocksData.animals[offset - 9];
        else if (offset >= 13 && offset <= 16) byteValue = globalUnlocksData.itemUnlocks[offset - 13];
        else if (offset == 17) byteValue = globalUnlocksData.relics;
        else if (offset >= 18 && offset <= 29) byteValue = globalUnlocksData.weapons[offset - 18];
        else if (offset >= 30 && offset <= 37) byteValue = globalUnlocksData.dlcWeapons[offset - 30];
        else if (offset >= 38 && offset <= 47) byteValue = globalUnlocksData.unknownArray2[offset - 38];
        else if (offset >= 48 && offset <= 51) byteValue = globalUnlocksData.backOffBarbarianTime[offset - 48];
        else if (offset == 52) byteValue = globalUnlocksData.backOffBarbarianBestChar;
        else if (offset >= 53 && offset <= 63) byteValue = globalUnlocksData.unknownArray3[offset - 53];
    }
    else {
        // Read from CharacterData
        uint32_t characterOffset = offset - 64;
        size_t characterIndex = characterOffset / 48;  // Each character occupies 48 bytes
        size_t fieldOffset = characterOffset % 48;

        if (characterIndex >= gameData["characters"].size()) {
            l.Get()->error("Invalid character index: {}", characterIndex);
            l.Get()->flush();
            throw std::runtime_error("Invalid character index");
        }

        l.Get()->info("Writing to offset {} for character ID: {}, value: {}", fieldOffset, characterIndex, byteValue);
        l.Get()->flush();

        CharacterData charData = jsonToCharacterData(gameData["characters"][characterIndex]);

        if (fieldOffset == 0) byteValue = charData.unlockStatus;
        else if (fieldOffset == 1) byteValue = charData.level;
        else if (fieldOffset >= 2 && fieldOffset <= 5) byteValue = charData.xp[fieldOffset - 2];
        else if (fieldOffset == 6) byteValue = charData.weapon;
        else if (fieldOffset == 7) byteValue = charData.pet;
        else if (fieldOffset >= 8 && fieldOffset <= 11) byteValue = charData.stats[fieldOffset - 8];
        else if (fieldOffset >= 12 && fieldOffset <= 14) byteValue = charData.normalLevelUnlocks[fieldOffset - 12];
        else if (fieldOffset >= 15 && fieldOffset <= 17) byteValue = charData.consumableItems[fieldOffset - 15];
        else if (fieldOffset == 18) byteValue = charData.nonConsumableItems;
        else if (fieldOffset >= 19 && fieldOffset <= 22) byteValue = charData.money[fieldOffset - 19];
        else if (fieldOffset == 23) byteValue = charData.insaneModeStoreUnlock;
        else if (fieldOffset >= 24 && fieldOffset <= 26) byteValue = charData.insaneLevelUnlocks[fieldOffset - 24];
        else if (fieldOffset == 27) byteValue = charData.skullStatus;
        else if (fieldOffset == 28) byteValue = charData.dugUpItems;
        else if (fieldOffset == 29) byteValue = charData.princessKisses;
        else if (fieldOffset >= 30 && fieldOffset <= 31) byteValue = charData.arenaWins[fieldOffset - 30];
        else if (fieldOffset >= 32 && fieldOffset <= 47) byteValue = charData.unknown[fieldOffset - 32];
    }

    return byteValue;
}

// The assembly code injection functions remain the same
__declspec(naked) void WriteInjectedCode() {
    __asm {
        cmp esi, 0xBB94
        jb jb_label
        mov eax, dword ptr ss : [ebp + 8]
        pop esi
        mov dword ptr ds : [eax] , 1
        mov dword ptr ds : [eax + 4] , 0
        pop ebp
        ret 0x0C
        jb_label :
        mov ecx, dword ptr ds : [edx + 4]
        mov al, byte ptr ss : [ebp + 0x10]
        mov byte ptr ds : [esi + ecx] , al
        push edx
        push ecx
        push eax
        push esi

        call WriteByteToJson

        pop esi
        pop eax
        pop ecx
        pop edx

        mov eax, dword ptr ss : [ebp + 8]
        inc dword ptr ds : [edx + 0x0C]
        pop esi
        mov dword ptr ds : [eax] , 0
        mov dword ptr ds : [eax + 4] , 0
        pop ebp
        ret 0x0C
    }
}

__declspec(naked) void ReadInjectedCode() {
    __asm {
        je je_label
        mov ecx, dword ptr ds : [edx + 0xC]
        cmp ecx, 0xBB94
        jb jb_label
        mov eax, dword ptr ds : [edx + 0xC]
        mov byte ptr ds : [eax] , 0
        je_label :
        mov eax, dword ptr ss : [ebp + 8]
        mov dword ptr ds : [eax] , 1
        mov dword ptr ds : [eax + 4] , 0
        pop ebp
        ret 0x0C
        jb_label :
        mov eax, dword ptr ds : [edx + 4]

        push ebx
        push eax
        push edx
        push esi
        push ecx // Offset

        call ReadByteFromJson
        mov bl, al

        pop ecx
        pop esi
        pop edx
        pop eax

        mov eax, dword ptr ss : [ebp + 0x10]
        mov byte ptr ds : [eax] , bl
        pop ebx
        mov eax, dword ptr ss : [ebp + 8]
        inc dword ptr ds : [edx + 0x0C]
        mov dword ptr ds : [eax] , 0
        mov dword ptr ds : [eax + 4] , 0
        pop ebp
        ret 0x0C
    }
}

__declspec(naked) void UnlockInjectedCode() {
    __asm {
        mov dword ptr ds : [eax + 0x0C] , ecx
        mov eax, dword ptr ss : [ebp + 8]
        mov dword ptr ds : [eax] , 0
        mov dword ptr ds : [eax + 4] , 0
        pop ebp
        ret 0x0C
    }
}

void InjectCode(uintptr_t base) {
    Logger& l = Logger::Instance();
    InitializeJsonFile();
    l.Get()->info("InjectCode(): Initialized JSON file");
    l.Get()->flush();

    DWORD oldProtect;
    // Injecting the write function
    unsigned char* writeTargetAddress = reinterpret_cast<unsigned char*>(base + 0x4AECC);

    VirtualProtect(writeTargetAddress, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

    *writeTargetAddress = 0xE9; // JMP opcode
    *reinterpret_cast<unsigned int*>(writeTargetAddress + 1) = (reinterpret_cast<unsigned char*>(&WriteInjectedCode) - writeTargetAddress - 5);

    VirtualProtect(writeTargetAddress, 5, oldProtect, &oldProtect);

    // Injecting the read function
    unsigned char* readTargetAddress = reinterpret_cast<unsigned char*>(base + 0x4AE4D);

    VirtualProtect(readTargetAddress, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

    *readTargetAddress = 0xE9;
    *reinterpret_cast<unsigned int*>(readTargetAddress + 1) = (reinterpret_cast<unsigned char*>(&ReadInjectedCode) - readTargetAddress - 5);

    VirtualProtect(readTargetAddress, 5, oldProtect, &oldProtect);

    // Injecting the unlock function
    unsigned char* unlockTargetAddress = reinterpret_cast<unsigned char*>(base + 0x4AF39);

    VirtualProtect(unlockTargetAddress, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

    *unlockTargetAddress = 0xE9;
    *reinterpret_cast<unsigned int*>(unlockTargetAddress + 1) = (reinterpret_cast<unsigned char*>(&UnlockInjectedCode) - unlockTargetAddress - 5);

    VirtualProtect(unlockTargetAddress, 5, oldProtect, &oldProtect);

    l.Get()->info("InjectCode(): Injected functions");
    l.Get()->flush();
}