#define NOMINMAX
#include "CustomSaveManager.h"
#include <windows.h>
#include <algorithm>
#include <cstring>
#include <shlobj.h>
#include <sstream>
#include <fstream>
#include <HookCrashers.h>

namespace CustomSave {

    int CustomSaveManager::NUM_BASE_CHARACTERS_LEGACY = 31;
    int CustomSaveManager::NUM_BASE_CHARACTERS = CustomSaveManager::ORIGINAL_BASE_CHAR_COUNT;
    int CustomSaveManager::TOTAL_GAME_CHARACTERS = 0;
    int CustomSaveManager::TOTAL_STREAMED_CHARACTERS = 0;
    int CustomSaveManager::TOTAL_GAME_SAVE_SIZE = 0;

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

    CustomSaveManager::CustomSaveManager() : m_firstTimeImportCompleted(false) {}

    bool CustomSaveManager::initialize(const std::string& modRootPath) {
        HookCrashers::LogInfo("[CustomSave] Initializing CustomSaveManager...");

        m_saveDirectoryPath = modRootPath + "CustomSave\\";
        m_iniFilePath = modRootPath + "CustomSave.ini";

        if (!CreateDirectoriesRecursive(m_saveDirectoryPath) ||
            !CreateDirectoriesRecursive(m_saveDirectoryPath + "base\\") ||
            !CreateDirectoriesRecursive(m_saveDirectoryPath + "addon\\") ||
            !CreateDirectoriesRecursive(m_saveDirectoryPath + "workshop\\")) {
            HookCrashers::LogError("[CustomSave] Failed to create save directories.");
            return false;
        }

        char valueBuffer[8];
        GetPrivateProfileStringA("Config", "FirstTimeSetupCompleted", "false", valueBuffer, sizeof(valueBuffer), m_iniFilePath.c_str());
        m_firstTimeImportCompleted = (strcmp(valueBuffer, "true") == 0);

        initializeCharacterList();

        if (m_firstTimeImportCompleted) {
            HookCrashers::LogInfo("[CustomSave] Custom save files found. Loading...");
            if (!loadAllSaveFiles()) {
                HookCrashers::LogError("[CustomSave] Failed to load existing custom save files.");
                return false;
            }
        }
        else {
            HookCrashers::LogInfo("[CustomSave] No custom save files found. Awaiting user decision...");
        }

        syncToGameBuffer();
        HookCrashers::LogInfo("[CustomSave] CustomSaveManager initialized successfully.");
        return true;
    }

    bool CustomSaveManager::isFirstTimeSetupNeeded() const {
        return !m_firstTimeImportCompleted;
    }

    void CustomSaveManager::markFirstTimeSetupAsComplete() {
        WritePrivateProfileStringA("Config", "FirstTimeSetupCompleted", "true", m_iniFilePath.c_str());
        m_firstTimeImportCompleted = true;
        HookCrashers::LogInfo("[CustomSave] First time setup has been marked as complete.");
    }

    void CustomSaveManager::triggerOriginalSaveImport() {
        HookCrashers::LogInfo("[CustomSave] User accepted import. Attempting to import captured game data...");
        const std::vector<uint8_t>& capturedData = HookCrashers::GetCapturedSaveData();

        if (capturedData.empty()) {
            HookCrashers::LogError("[CustomSave] IMPORT FAILED: No save data was captured from the game.");
            declineOriginalSaveImport();
            return;
        }

        importAndSaveData(capturedData);
    }

    void CustomSaveManager::declineOriginalSaveImport() {
        HookCrashers::LogInfo("[CustomSave] User declined import or import failed. Creating fresh save files.");
        generateDefaultSaveData();
        saveAllSaveFiles();
        markFirstTimeSetupAsComplete();
        syncToGameBuffer();
    }

    void CustomSaveManager::importAndSaveData(const std::vector<uint8_t>& decryptedData) {
        HookCrashers::LogInfo("[CustomSave] Decrypted data received. Saving to JSON files...");

        if (decryptedData.size() < sizeof(GlobalData)) {
            HookCrashers::LogError("[CustomSave] Decrypted data is too small for global data.");
            declineOriginalSaveImport();
            return;
        }

        std::memcpy(&m_globalData, decryptedData.data(), sizeof(GlobalData));
        saveGlobalData();
        HookCrashers::LogInfo("[CustomSave] Global data imported and saved.");

        HookCrashers::LogInfo("[CustomSave] Importing " + std::to_string(ORIGINAL_BASE_CHAR_COUNT) + " original base characters...");
        for (int i = 0; i < ORIGINAL_BASE_CHAR_COUNT; ++i) {
            size_t offset = sizeof(GlobalData) + (i * sizeof(CharacterData));
            if (offset + sizeof(CharacterData) > decryptedData.size()) {
                HookCrashers::LogWarn("[CustomSave] Save data ended unexpectedly. Imported " + std::to_string(i) + " base characters.");
                break;
            }
            if (static_cast<size_t>(i) < m_characters.size()) {
                std::memcpy(&m_characters[i].data, decryptedData.data() + offset, sizeof(CharacterData));
                saveCharacterData(m_characters[i].id, "base", i);
            }
        }

        size_t workshopDataStartOffset = sizeof(GlobalData) + (NUM_BASE_CHARACTERS_LEGACY * sizeof(CharacterData));
        HookCrashers::LogInfo("[CustomSave] Importing Workshop characters...");
        size_t workshop_start_index_in_array = NUM_BASE_CHARACTERS;

        for (int i = 0; i < NUM_WORKSHOP_CHARACTERS; ++i) {
            size_t source_offset = workshopDataStartOffset + (i * sizeof(CharacterData));
            if (source_offset + sizeof(CharacterData) > decryptedData.size()) {
                HookCrashers::LogWarn("[CustomSave] Save data ended before all workshop slots. Imported " + std::to_string(i) + " slots.");
                break;
            }

            size_t dest_base_index = workshop_start_index_in_array + i;
            if (dest_base_index < m_characters.size()) {
                std::memcpy(&m_characters[dest_base_index].data, decryptedData.data() + source_offset, sizeof(CharacterData));
            }
        }
        for (int i = 1; i <= NUM_WORKSHOP_CHARACTERS; ++i) {
            saveWorkshopData(i);
        }
        HookCrashers::LogInfo("[CustomSave] Workshop data imported and saved.");

        HookCrashers::LogInfo("[CustomSave] Creating default save files for addon characters...");
        for (size_t i = ORIGINAL_BASE_CHAR_COUNT; i < static_cast<size_t>(NUM_BASE_CHARACTERS); ++i) {
            saveCharacterData(m_characters[i].id, "addon", i);
        }

        markFirstTimeSetupAsComplete();
        syncToGameBuffer();
        HookCrashers::LogInfo("[CustomSave] Import completed successfully.");
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

        std::ifstream iniFile(m_iniFilePath);
        if (iniFile.is_open()) {
            std::string line;
            bool inAddonSection = false;
            int addonCount = 0;
            while (std::getline(iniFile, line)) {
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t\r\n") + 1);
                if (line == "[AddonCharacters]") {
                    inAddonSection = true; continue;
                }
                if (!line.empty() && line[0] == '[' && line != "[AddonCharacters]") {
                    inAddonSection = false; continue;
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
                            m_characters.push_back(createEntry(parts[0], static_cast<uint8_t>(std::stoi(parts[1])), static_cast<uint8_t>(std::stoi(parts[2])), (parts[3] == "true")));
                            addonCount++;
                        }
                        catch (const std::exception& e) {
                            HookCrashers::LogError("[CustomSave] Error parsing addon character line: " + line + " - " + e.what());
                        }
                    }
                    else {
                        HookCrashers::LogError("[CustomSave] Invalid addon character line: " + line);
                    }
                }
            }
            iniFile.close();
            HookCrashers::LogInfo("[CustomSave] Loaded " + std::to_string(addonCount) + " addon characters.");
        }
        else {
            HookCrashers::LogWarn("[CustomSave] Could not open INI file: " + m_iniFilePath);
        }

        NUM_BASE_CHARACTERS = m_characters.size();
        TOTAL_GAME_CHARACTERS = NUM_BASE_CHARACTERS + (NUM_WORKSHOP_CHARACTERS * 4);
        TOTAL_STREAMED_CHARACTERS = NUM_BASE_CHARACTERS + (NUM_WORKSHOP_CHARACTERS * 4);
        TOTAL_GAME_SAVE_SIZE = NUM_GLOBAL_BYTES + (TOTAL_GAME_CHARACTERS * NUM_CHARACTER_BYTES);
        m_gameSaveBuffer.resize(TOTAL_GAME_SAVE_SIZE, 0);

        for (int i = 0; i < NUM_WORKSHOP_CHARACTERS; ++i) {
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

        for (auto& character : m_characters) {
            character.data = {};
            character.data.unlocked = 0x80;
            character.data.weapon = 0;
            character.data.animal = 0;
            character.data.strength = 1;
            character.data.defense = 1;
            character.data.magic = 1;
            character.data.agility = 1;
        }
    }

    bool CustomSaveManager::loadAllSaveFiles() {
        if (!loadGlobalData()) { saveGlobalData(); }
        for (size_t i = 0; i < static_cast<size_t>(NUM_BASE_CHARACTERS); ++i) {
            std::string subfolder = (i < ORIGINAL_BASE_CHAR_COUNT) ? "base" : "addon";
            if (!loadCharacterData(m_characters[i].id, subfolder, i)) {
                saveCharacterData(m_characters[i].id, subfolder, i);
            }
        }
        for (int i = 1; i <= NUM_WORKSHOP_CHARACTERS; ++i) {
            if (!loadWorkshopData(i)) { saveWorkshopData(i); }
        }
        return true;
    }

    bool CustomSaveManager::loadGlobalData() {
        std::string filePath = m_saveDirectoryPath + "globalUnlocks.json";
        std::ifstream file(filePath);
        if (!file.is_open()) return false;
        try { nlohmann::json root; file >> root; m_globalData = root.get<GlobalData>(); return true; }
        catch (const std::exception& e) { HookCrashers::LogError("[CustomSave] Failed to parse global data: " + std::string(e.what())); return false; }
    }

    bool CustomSaveManager::loadCharacterData(const std::string& charId, const std::string& subfolder, size_t index) {
        std::string filePath = m_saveDirectoryPath + subfolder + "\\" + charId + ".json";
        std::ifstream file(filePath);
        if (!file.is_open()) return false;
        try { nlohmann::json root; file >> root; if (index < m_characters.size()) m_characters[index].data = root.get<CharacterData>(); return true; }
        catch (const std::exception& e) { HookCrashers::LogError("[CustomSave] Failed to parse character data for " + charId + ": " + std::string(e.what())); return false; }
    }

    bool CustomSaveManager::loadWorkshopData(int workshopId) {
        std::string filePath = m_saveDirectoryPath + "workshop\\workshop" + std::to_string(workshopId) + ".json";
        std::ifstream file(filePath);
        if (!file.is_open()) return false;
        try {
            nlohmann::json root;
            file >> root;
            CharacterData workshopChar = root.get<CharacterData>();
            size_t baseIndex = NUM_BASE_CHARACTERS + (workshopId - 1);
            if (baseIndex < m_characters.size()) {
                m_characters[baseIndex].data = workshopChar;
            }
            return true;
        }
        catch (const std::exception& e) { HookCrashers::LogError("[CustomSave] Failed to parse workshop data " + std::to_string(workshopId) + ": " + std::string(e.what())); return false; }
    }

    bool CustomSaveManager::saveAllSaveFiles() {
        if (!saveGlobalData()) return false;
        for (size_t i = 0; i < static_cast<size_t>(NUM_BASE_CHARACTERS); ++i) {
            std::string subfolder = (i < ORIGINAL_BASE_CHAR_COUNT) ? "base" : "addon";
            saveCharacterData(m_characters[i].id, subfolder, i);
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
        size_t baseIndex = NUM_BASE_CHARACTERS + (workshopId - 1);
        if (baseIndex < m_characters.size()) {
            nlohmann::json root = m_characters[baseIndex].data;
            file << root.dump(2);
        }
        return true;
    }

    void CustomSaveManager::commitChangesToDisk() {
        syncFromGameBuffer();
        saveAllSaveFiles();
    }

    void CustomSaveManager::syncToGameBuffer() {
        if (m_gameSaveBuffer.empty()) return;
        std::memcpy(m_gameSaveBuffer.data(), &m_globalData, NUM_GLOBAL_BYTES);

        for (size_t i = 0; i < m_characters.size(); ++i) {
            if (i >= static_cast<size_t>(TOTAL_GAME_CHARACTERS)) break;
            size_t offset = NUM_GLOBAL_BYTES + (i * NUM_CHARACTER_BYTES);
            if (offset + NUM_CHARACTER_BYTES <= m_gameSaveBuffer.size()) {
                std::memcpy(m_gameSaveBuffer.data() + offset, &m_characters[i].data, NUM_CHARACTER_BYTES);
            }
        }
    }

    void CustomSaveManager::syncFromGameBuffer() {
        if (m_gameSaveBuffer.empty()) return;
        std::memcpy(&m_globalData, m_gameSaveBuffer.data(), NUM_GLOBAL_BYTES);

        size_t workshop_start_index_in_array = NUM_BASE_CHARACTERS;
        for (size_t i = 0; i < m_characters.size(); ++i) {
            if (i >= static_cast<size_t>(TOTAL_GAME_CHARACTERS)) break;
            size_t offset = NUM_GLOBAL_BYTES + (i * NUM_CHARACTER_BYTES);
            if (offset + NUM_CHARACTER_BYTES <= m_gameSaveBuffer.size()) {
                std::memcpy(&m_characters[i].data, m_gameSaveBuffer.data() + offset, NUM_CHARACTER_BYTES);
            }
        }
        for (size_t i = 0; i < NUM_WORKSHOP_CHARACTERS; ++i) {
            size_t base_index = workshop_start_index_in_array + i;
            if (base_index + 3 < m_characters.size()) {
                m_characters[base_index + 1].data = m_characters[base_index].data;
                m_characters[base_index + 2].data = m_characters[base_index].data;
                m_characters[base_index + 3].data = m_characters[base_index].data;
            }
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