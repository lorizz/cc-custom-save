#pragma once
#include "../Data/GlobalData.h"
#include "../Data/CharacterData.h"
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <json.hpp>

namespace CustomSave {
    struct CustomCharacterEntry {
        CharacterData data;
        std::string id;
    };

    class CustomSaveManager {
    public:
        static constexpr int ORIGINAL_BASE_CHAR_COUNT = 32;
        static constexpr int NUM_BASE_CHARACTERS_LEGACY = 31;
        static constexpr int NUM_WORKSHOP_CHARACTERS = 10;
        static constexpr int NUM_GLOBAL_BYTES = sizeof(GlobalData);
        static constexpr int NUM_CHARACTER_BYTES = sizeof(CharacterData);

        static int NUM_BASE_CHARACTERS;
        static int TOTAL_GAME_CHARACTERS;
        static int TOTAL_STREAMED_CHARACTERS;
        static int TOTAL_GAME_SAVE_SIZE;

    private:
        CustomSaveManager();
        CustomSaveManager(const CustomSaveManager&) = delete;
        CustomSaveManager& operator=(const CustomSaveManager&) = delete;

        std::string m_saveDirectoryPath;
        std::string m_iniFilePath;
        GlobalData m_globalData;
        std::vector<CustomCharacterEntry> m_characters;
        std::vector<uint8_t> m_gameSaveBuffer;

        void initializeCharacterList();
        bool loadAllSaveFiles();
        bool saveAllSaveFiles();
        bool loadGlobalData();
        bool saveGlobalData();
        bool loadCharacterData(const std::string& charId, const std::string& subfolder, size_t index);
        bool saveCharacterData(const std::string& charId, const std::string& subfolder, size_t index);
        bool loadWorkshopData(int workshopId);
        bool saveWorkshopData(int workshopId);
        void generateDefaultSaveData();
        void syncToGameBuffer();
        void syncFromGameBuffer();

    public:
        static CustomSaveManager& getInstance();
        bool initialize(const std::string& modRootPath);
        uint8_t readByteFromGameBuffer(size_t offset);
        void writeByteToGameBuffer(size_t offset, uint8_t value);
        void commitChangesToDisk();
        const CustomCharacterEntry& getCharacterEntry(size_t index) const;
    };
}

namespace nlohmann {
    template <>
    struct adl_serializer<CustomSave::CustomCharacterEntry> {
        static void to_json(json& j, const CustomSave::CustomCharacterEntry& entry) {
            j = { {"id", entry.id}, {"data", entry.data} };
        }
        static void from_json(const json& j, CustomSave::CustomCharacterEntry& entry) {
            j.at("id").get_to(entry.id);
            j.at("data").get_to(entry.data);
        }
    };
}