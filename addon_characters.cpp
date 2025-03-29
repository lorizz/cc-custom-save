#include "addon_characters.h"
#include "logger.h"
#include "utils.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>

std::vector<CharacterData> addonCharacters;

std::vector<CharacterData> readAddonCharacters(const std::string& iniFilename) {
    std::vector<CharacterData> addonChars;
    std::ifstream iniFile(iniFilename);
    Logger& logger = Logger::Instance();

    if (!iniFile.is_open()) {
        logger.Get()->error("Cannot open INI file: {}", iniFilename);
        return addonChars;
    }

    std::string line;
    bool inAddonSection = false;
    while (std::getline(iniFile, line)) {
        if (line.empty())
            continue;
        if (line[0] == ';' || line[0] == '#')
            continue;

        if (!inAddonSection) {
            if (line.find("[AddonCharacters]") != std::string::npos) {
                inAddonSection = true;
            }
            continue;
        }
        if (line[0] == '[')
            break;

        std::istringstream iss(line);
        std::string token;
        std::vector<std::string> tokens;
        while (std::getline(iss, token, ',')) {
            tokens.push_back(trim(token));
        }

        if (tokens.size() != 3) {
            logger.Get()->error("Malformed addon character line: {}", line);
            continue;
        }

        std::string id = tokens[0];
        uint8_t startingWeaponId = static_cast<uint8_t>(std::stoi(tokens[1]));
        bool unlocked = (tokens[2] == "true" || tokens[2] == "1");

        CharacterData character(id, startingWeaponId, unlocked);

        character.addData("isUnlocked", std::vector<uint8_t>{ unlocked });

        addonChars.push_back(character);
        logger.Get()->info("Parsed addon character: id='{}', startingWeaponId={}, isUnlocked={}",
            id, startingWeaponId, unlocked);
    }

    iniFile.close();
    logger.Get()->info("Found {} addon characters", addonChars.size());
    return addonChars;
}

void InitializeAddonCharacters(const std::string& iniFilename) {
    addonCharacters = readAddonCharacters(iniFilename);
}
