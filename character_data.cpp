#include "character_data.h"

extern std::vector<FieldInfo> gCharacterStructure;

// Constructor
CharacterData::CharacterData(const std::string& characterId, uint8_t weapon, bool initiallyUnlocked)
    : id(characterId) {
    registerCharacter(initiallyUnlocked ? 0x80 : 0x00, weapon);
}

// Add data to the character
void CharacterData::addData(const std::string& key, const std::vector<uint8_t>& value) {
    data[key] = value;
}

// Register a character with unique id, unlockStatus, and weapon
void CharacterData::registerCharacter(uint8_t unlockStatus, uint8_t weapon) {
    for (const auto& field : gCharacterStructure) {
        std::vector<uint8_t> defaultValue(field.length, 0x00);
        if (field.name == "unlockStatus") {
            defaultValue[0] = unlockStatus;
        }
        else if (field.name == "weapon") {
            defaultValue[0] = weapon;
        }
        else if (field.name == "stats") {
            std::fill(defaultValue.begin(), defaultValue.end(), 0x01);
        }
        addData(field.name, defaultValue);
    }
}