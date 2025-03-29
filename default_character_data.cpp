#include "default_character_data.h"

// Global variable definition.
// Initially empty. It will be set up after the INI structure is loaded.
std::vector<CharacterData> defaultCharacters;

std::vector<CharacterData> InitializeDefaultCharacters() {
    std::vector<CharacterData> characters;
    characters.push_back(CharacterData("greenKnight", 0x03, true));
    characters.push_back(CharacterData("redKnight", 0x19, true));
    characters.push_back(CharacterData("blueKnight", 0x27, true));
    characters.push_back(CharacterData("orangeKnight", 0x38, true));
    characters.push_back(CharacterData("grayKnight", 0x02));
    characters.push_back(CharacterData("barbarian", 0x13));
    characters.push_back(CharacterData("thief", 0x0A));
    characters.push_back(CharacterData("fencer", 0x12));
    characters.push_back(CharacterData("beekeeper", 0x22));
    characters.push_back(CharacterData("industrialist", 0x1B));
    characters.push_back(CharacterData("alien", 0x2F));
    characters.push_back(CharacterData("king", 0x24));
    characters.push_back(CharacterData("brute", 0x0C));
    characters.push_back(CharacterData("snakey", 0x21));
    characters.push_back(CharacterData("saracen", 0x0F));
    characters.push_back(CharacterData("royalGuard", 0x0F));
    characters.push_back(CharacterData("stoveface", 0x06));
    characters.push_back(CharacterData("peasant", 0x2D));
    characters.push_back(CharacterData("bear", 0x1A));
    characters.push_back(CharacterData("necromancer", 0x40));
    characters.push_back(CharacterData("conehead", 0x2B));
    characters.push_back(CharacterData("civilian", 0x14));
    characters.push_back(CharacterData("openGrayKnight", 0x02));
    characters.push_back(CharacterData("fireDemon", 0x23));
    characters.push_back(CharacterData("skeleton", 0x1F));
    characters.push_back(CharacterData("iceskimo", 0x30));
    characters.push_back(CharacterData("ninja", 0x32));
    characters.push_back(CharacterData("cultist", 0x44));
    characters.push_back(CharacterData("pinkKnight", 0x3F));
    characters.push_back(CharacterData("blacksmith", 0x55));
    characters.push_back(CharacterData("hatty", 0x54));

    return characters;
}
