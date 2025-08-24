#define NOMINMAX
#include "CustomSaveManager.h"
#include <HookCrashersAPI.h> 
#include <windows.h>
#include <algorithm>
#include <cstring>
#include <shlobj.h>
#include <sstream>

int CustomSave::CustomSaveManager::NUM_BASE_CHARACTERS = CustomSave::CustomSaveManager::ORIGINAL_BASE_CHAR_COUNT;
int CustomSave::CustomSaveManager::TOTAL_GAME_CHARACTERS = 0;
int CustomSave::CustomSaveManager::TOTAL_STREAMED_CHARACTERS = 0;
int CustomSave::CustomSaveManager::TOTAL_GAME_SAVE_SIZE = 0;

namespace CustomSave {

    bool CreateDirectoriesRecursive(const std::string& path) {
        if (path.empty()) return true;
        if (SHCreateDirectoryExA(NULL, path.c_str(), NULL) == ERROR_SUCCESS) return true;
        DWORD err = GetLastError();
        return (err == ERROR_FILE_EXISTS || err == ERROR_ALREADY_EXISTS);
    }

    CustomSaveManager& CustomSaveManager::getInstance() {
        static CustomSaveManager instance;
        return instance;
    }

    CustomSaveManager::CustomSaveManager() {}

    bool CustomSaveManager::initialize(const std::string& modRootPath) {
        m_saveDirectoryPath = modRootPath + "CustomSave\\";
        m_iniFilePath = modRootPath + "CustomSave.ini";

        if (!CreateDirectoriesRecursive(m_saveDirectoryPath)) {
            HookCrashers::API::Client::LogError("[CustomSave] Failed to create save directory: " + m_saveDirectoryPath);
            return false;
        }

        if (!CreateDirectoriesRecursive(m_saveDirectoryPath + "base\\")) {
            HookCrashers::API::Client::LogError("[CustomSave] Failed to create base directory");
            return false;
        }

        if (!CreateDirectoriesRecursive(m_saveDirectoryPath + "addon\\")) {
            HookCrashers::API::Client::LogError("[CustomSave] Failed to create addon directory");
            return false;
        }

        if (!CreateDirectoriesRecursive(m_saveDirectoryPath + "workshop\\")) {
            HookCrashers::API::Client::LogError("[CustomSave] Failed to create workshop directory");
            return false;
        }

        generateDefaultSaveData();

        if (!loadAllSaveFiles()) {
            return false;
        }

        syncToGameBuffer();
        return true;
    }

    void CustomSaveManager::initializeCharacterList() {
        m_characters.clear();

        auto createEntry = [&](const std::string& logical_id, uint8_t weapon_id, uint8_t pet_id, bool unlocked) {
            CustomCharacterEntry entry;
            entry.id = logical_id;
            entry.data = {};
            entry.data.unlocked = unlocked ? 0x80 : 0x00;
            entry.data.weapon = weapon_id;
            entry.data.animal = pet_id;
            entry.data.strength = 1;
            entry.data.defense = 1;
            entry.data.magic = 1;
            entry.data.agility = 1;
            return entry;
            };

        m_characters.push_back(createEntry("greenKnight", 0x03, 0, true));
        m_characters.push_back(createEntry("redKnight", 0x19, 0, true));
        m_characters.push_back(createEntry("blueKnight", 0x27, 0, true));
        m_characters.push_back(createEntry("orangeKnight", 0x38, 0, true));
        m_characters.push_back(createEntry("grayKnight", 0x02, 0, true));
        m_characters.push_back(createEntry("barbarian", 0x13, 0, true));
        m_characters.push_back(createEntry("thief", 0x0A, 0, true));
        m_characters.push_back(createEntry("fencer", 0x12, 0, true));
        m_characters.push_back(createEntry("beekeeper", 0x22, 0, true));
        m_characters.push_back(createEntry("industrialist", 0x1B, 0, true));
        m_characters.push_back(createEntry("alien", 0x2F, 0, true));
        m_characters.push_back(createEntry("king", 0x24, 0, true));
        m_characters.push_back(createEntry("brute", 0x0C, 0, true));
        m_characters.push_back(createEntry("snakey", 0x21, 0, true));
        m_characters.push_back(createEntry("saracen", 0x0F, 0, true));
        m_characters.push_back(createEntry("royalGuard", 0x0F, 0, true));
        m_characters.push_back(createEntry("stoveface", 0x06, 0, true));
        m_characters.push_back(createEntry("peasant", 0x2D, 0, true));
        m_characters.push_back(createEntry("bear", 0x1A, 0, true));
        m_characters.push_back(createEntry("necromancer", 0x40, 0, true));
        m_characters.push_back(createEntry("conehead", 0x2B, 0, true));
        m_characters.push_back(createEntry("civilian", 0x14, 0, true));
        m_characters.push_back(createEntry("openGrayKnight", 0x02, 0, true));
        m_characters.push_back(createEntry("fireDemon", 0x23, 0, true));
        m_characters.push_back(createEntry("skeleton", 0x1F, 0, true));
        m_characters.push_back(createEntry("iceskimo", 0x30, 0, true));
        m_characters.push_back(createEntry("ninja", 0x32, 0, true));
        m_characters.push_back(createEntry("cultist", 0x44, 0, true));
        m_characters.push_back(createEntry("pinkKnight", 0x3F, 0, true));
        m_characters.push_back(createEntry("blacksmith", 0x55, 0, true));
        m_characters.push_back(createEntry("hatty", 0x54, 0, true));
        m_characters.push_back(createEntry("painterBoss", 0x56, 0, true));

        HookCrashers::API::Client::LogInfo("[CustomSave] Reading addon characters from: " + m_iniFilePath);

        std::ifstream iniFile(m_iniFilePath);
        if (iniFile.is_open()) {
            std::string line;
            bool inAddonSection = false;
            int addonCount = 0;

            while (std::getline(iniFile, line)) {
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);

                if (line == "[AddonCharacters]") {
                    inAddonSection = true;
                    continue;
                }

                if (!line.empty() && line[0] == '[' && line != "[AddonCharacters]") {
                    inAddonSection = false;
                    continue;
                }

                if (inAddonSection && !line.empty() && line[0] != ';' && line[0] != '#') {
                    std::stringstream ss(line);
                    std::string part;
                    std::vector<std::string> parts;

                    while (std::getline(ss, part, ',')) {
                        part.erase(0, part.find_first_not_of(" \t"));
                        part.erase(part.find_last_not_of(" \t") + 1);
                        parts.push_back(part);
                    }

                    if (parts.size() == 4) {
                        try {
                            std::string charId = parts[0];
                            uint8_t weapon = static_cast<uint8_t>(std::stoi(parts[1]));
                            uint8_t pet = static_cast<uint8_t>(std::stoi(parts[2]));
                            bool unlocked = (parts[3] == "true");

                            m_characters.push_back(createEntry(charId, weapon, pet, unlocked));
                            addonCount++;
                            HookCrashers::API::Client::LogInfo("[CustomSave] Loaded addon character: " + charId);
                        }
                        catch (const std::exception& e) {
                            HookCrashers::API::Client::LogError("[CustomSave] Error parsing addon character line: " + line + " - " + e.what());
                        }
                    }
                    else {
                        HookCrashers::API::Client::LogError("[CustomSave] Invalid addon character line (expected 4 parameters): " + line);
                    }
                }
            }

            iniFile.close();
            HookCrashers::API::Client::LogInfo("[CustomSave] Loaded " + std::to_string(addonCount) + " addon characters");
        }
        else {
            HookCrashers::API::Client::LogError("[CustomSave] Could not open INI file: " + m_iniFilePath);
        }

        NUM_BASE_CHARACTERS = m_characters.size();
        TOTAL_GAME_CHARACTERS = NUM_BASE_CHARACTERS + NUM_WORKSHOP_CHARACTERS;
        TOTAL_STREAMED_CHARACTERS = NUM_BASE_CHARACTERS + (NUM_WORKSHOP_CHARACTERS * 4);
        TOTAL_GAME_SAVE_SIZE = NUM_GLOBAL_BYTES + (TOTAL_GAME_CHARACTERS * NUM_CHARACTER_BYTES);
        m_gameSaveBuffer.resize(TOTAL_GAME_SAVE_SIZE, 0);
        HookCrashers::API::Client::LogInfo("[CustomSave] Total base characters (incl. addons): " + std::to_string(NUM_BASE_CHARACTERS));

        for (int i = 0; i < (NUM_WORKSHOP_CHARACTERS * 4); ++i) {
            m_characters.push_back(createEntry("workshop" + std::to_string(i), 0, 0, true));
        }
    }

    void CustomSaveManager::generateDefaultSaveData() {
        m_globalData = {};
        m_globalData.vibration = 1;
        m_globalData.voice_volume = 100;
        m_globalData.music_volume = 75;
        m_globalData.sfx_volume = 80;
        m_globalData.gore = 1;

        initializeCharacterList();
    }

    bool CustomSaveManager::loadAllSaveFiles() {
        if (!loadGlobalData()) {
            HookCrashers::API::Client::LogInfo("[CustomSave] Global data not found, creating default");
            saveGlobalData();
        }

        for (size_t i = 0; i < ORIGINAL_BASE_CHAR_COUNT; ++i) {
            if (i < m_characters.size()) {
                if (!loadCharacterData(m_characters[i].id, "base", i)) {
                    HookCrashers::API::Client::LogInfo("[CustomSave] Creating default data for base character: " + m_characters[i].id);
                    saveCharacterData(m_characters[i].id, "base", i);
                }
            }
        }

        for (size_t i = ORIGINAL_BASE_CHAR_COUNT; i < NUM_BASE_CHARACTERS; ++i) {
            if (i < m_characters.size()) {
                if (!loadCharacterData(m_characters[i].id, "addon", i)) {
                    HookCrashers::API::Client::LogInfo("[CustomSave] Creating default data for addon character: " + m_characters[i].id);
                    saveCharacterData(m_characters[i].id, "addon", i);
                }
            }
        }

        for (int i = 1; i <= NUM_WORKSHOP_CHARACTERS; ++i) {
            if (!loadWorkshopData(i)) {
                HookCrashers::API::Client::LogInfo("[CustomSave] Creating default workshop data for workshop" + std::to_string(i));
                saveWorkshopData(i);
            }
        }

        return true;
    }

    bool CustomSaveManager::loadGlobalData() {
        std::string filePath = m_saveDirectoryPath + "globalUnlocks.json";
        std::ifstream file(filePath);
        if (!file.is_open()) return false;

        try {
            nlohmann::json root;
            file >> root;
            m_globalData = root.get<GlobalData>();
            return true;
        }
        catch (const std::exception& e) {
            HookCrashers::API::Client::LogError("[CustomSave] Failed to parse global data: " + std::string(e.what()));
            return false;
        }
    }

    bool CustomSaveManager::loadCharacterData(const std::string& charId, const std::string& subfolder, size_t index) {
        std::string filePath = m_saveDirectoryPath + subfolder + "\\" + charId + ".json";
        std::ifstream file(filePath);
        if (!file.is_open()) return false;

        try {
            nlohmann::json root;
            file >> root;
            if (index < m_characters.size()) {
                m_characters[index].data = root.get<CharacterData>();
            }
            return true;
        }
        catch (const std::exception& e) {
            HookCrashers::API::Client::LogError("[CustomSave] Failed to parse character data for " + charId + ": " + std::string(e.what()));
            return false;
        }
    }

    bool CustomSaveManager::loadWorkshopData(int workshopId) {
        std::string filePath = m_saveDirectoryPath + "workshop\\workshop" + std::to_string(workshopId) + ".json";
        std::ifstream file(filePath);
        if (!file.is_open()) return false;

        try {
            nlohmann::json root;
            file >> root;
            std::vector<CharacterData> workshopChars = root.get<std::vector<CharacterData>>();

            size_t startIndex = NUM_BASE_CHARACTERS + ((workshopId - 1) * 4);
            for (size_t i = 0; i < 4 && i < workshopChars.size(); ++i) {
                size_t charIndex = startIndex + i;
                if (charIndex < m_characters.size()) {
                    m_characters[charIndex].data = workshopChars[i];
                }
            }
            return true;
        }
        catch (const std::exception& e) {
            HookCrashers::API::Client::LogError("[CustomSave] Failed to parse workshop data " + std::to_string(workshopId) + ": " + std::string(e.what()));
            return false;
        }
    }

    bool CustomSaveManager::saveAllSaveFiles() {
        if (!saveGlobalData()) return false;

        for (size_t i = 0; i < ORIGINAL_BASE_CHAR_COUNT; ++i) {
            if (i < m_characters.size()) {
                saveCharacterData(m_characters[i].id, "base", i);
            }
        }

        for (size_t i = ORIGINAL_BASE_CHAR_COUNT; i < NUM_BASE_CHARACTERS; ++i) {
            if (i < m_characters.size()) {
                saveCharacterData(m_characters[i].id, "addon", i);
            }
        }

        for (int i = 1; i <= NUM_WORKSHOP_CHARACTERS; ++i) {
            saveWorkshopData(i);
        }

        return true;
    }

    bool CustomSaveManager::saveGlobalData() {
        std::string filePath = m_saveDirectoryPath + "globalUnlocks.json";
        std::ofstream file(filePath);
        if (!file.is_open()) return false;

        nlohmann::json root = m_globalData;
        file << root.dump(2);
        return true;
    }

    bool CustomSaveManager::saveCharacterData(const std::string& charId, const std::string& subfolder, size_t index) {
        std::string filePath = m_saveDirectoryPath + subfolder + "\\" + charId + ".json";
        std::ofstream file(filePath);
        if (!file.is_open()) return false;

        if (index < m_characters.size()) {
            nlohmann::json root = m_characters[index].data;
            file << root.dump(2);
        }
        return true;
    }

    bool CustomSaveManager::saveWorkshopData(int workshopId) {
        std::string filePath = m_saveDirectoryPath + "workshop\\workshop" + std::to_string(workshopId) + ".json";
        std::ofstream file(filePath);
        if (!file.is_open()) return false;

        std::vector<CharacterData> workshopChars;
        size_t startIndex = NUM_BASE_CHARACTERS + ((workshopId - 1) * 4);
        for (size_t i = 0; i < 4; ++i) {
            size_t charIndex = startIndex + i;
            if (charIndex < m_characters.size()) {
                workshopChars.push_back(m_characters[charIndex].data);
            }
        }

        nlohmann::json root = workshopChars;
        file << root.dump(2);
        return true;
    }

    void CustomSaveManager::commitChangesToDisk() {
        syncFromGameBuffer();
        saveAllSaveFiles();
    }

    void CustomSaveManager::syncToGameBuffer() {
        std::memcpy(m_gameSaveBuffer.data(), &m_globalData, NUM_GLOBAL_BYTES);
        size_t charsToCopy = std::min((size_t)TOTAL_GAME_CHARACTERS, m_characters.size());
        for (size_t i = 0; i < charsToCopy; ++i) {
            size_t offset = NUM_GLOBAL_BYTES + (i * NUM_CHARACTER_BYTES);
            std::memcpy(m_gameSaveBuffer.data() + offset, &m_characters[i].data, NUM_CHARACTER_BYTES);
        }
    }

    void CustomSaveManager::syncFromGameBuffer() {
        std::memcpy(&m_globalData, m_gameSaveBuffer.data(), NUM_GLOBAL_BYTES);
        for (size_t i = 0; i < TOTAL_GAME_CHARACTERS; ++i) {
            if (i >= m_characters.size()) continue;
            size_t offset = NUM_GLOBAL_BYTES + (i * NUM_CHARACTER_BYTES);
            std::memcpy(&m_characters[i].data, m_gameSaveBuffer.data() + offset, NUM_CHARACTER_BYTES);
        }
    }

    uint8_t CustomSaveManager::readByteFromGameBuffer(size_t offset) {
        if (offset >= m_gameSaveBuffer.size()) return 0;
        return m_gameSaveBuffer[offset];
    }

    void CustomSaveManager::writeByteToGameBuffer(size_t offset, uint8_t value) {
        if (offset >= m_gameSaveBuffer.size()) return;
        m_gameSaveBuffer[offset] = value;
    }

    const CustomCharacterEntry& CustomSaveManager::getCharacterEntry(size_t index) const {
        if (index >= m_characters.size()) {
            static const CustomCharacterEntry dummy_entry = {};
            return dummy_entry;
        }
        return m_characters[index];
    }
}