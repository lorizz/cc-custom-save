#pragma once
#include <string>
#include <map>
#include <vector>
#include "dynamic_character_data.h"

class CharacterData {
private:
    std::string id;
    std::map<std::string, std::vector<uint8_t>> data;
    void registerCharacter(uint8_t unlockStatus, uint8_t weapon);
public:
    CharacterData(const std::string& characterId, uint8_t weapon, bool initiallyUnlocked = false);
    void addData(const std::string& key, const std::vector<uint8_t>& value);
    const std::string& getId() const { return id; }
    const std::map<std::string, std::vector<uint8_t>>& getData() const { return data; }
};
