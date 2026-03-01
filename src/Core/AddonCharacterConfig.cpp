#include "AddonCharacterConfig.h"
#include <fstream>
#include <sstream>

AddonCharacterConfig& AddonCharacterConfig::getInstance() {
    static AddonCharacterConfig instance;
    return instance;
}

bool AddonCharacterConfig::loadFromIni(const std::string& iniPath) {
    std::ifstream file(iniPath);
    if (!file.is_open()) return false;

    m_addons.clear();
    std::string line;
    bool inSection = false;

    while (std::getline(file, line)) {
        // trim
        line.erase(0, line.find_first_not_of(" \t"));
        if (line.empty() || line[0] == ';' || line[0] == '#') continue;

        if (line == "[AddonCharacters]") { inSection = true; continue; }
        if (line[0] == '[') { inSection = false; continue; }
        if (!inSection) continue;

        // format: id, weapon, animal, unlocked, freshOnly
        std::stringstream ss(line);
        std::string part;
        std::vector<std::string> parts;
        while (std::getline(ss, part, ',')) {
            part.erase(0, part.find_first_not_of(" \t"));
            part.erase(part.find_last_not_of(" \t\r\n") + 1);
            parts.push_back(part);
        }
        if (parts.size() < 5) continue;

        AddonCharacterDef def;
        def.id = parts[0];
        def.weapon = (uint8_t)std::stoi(parts[1]);
        def.animal = (uint8_t)std::stoi(parts[2]);
        def.unlocked = parts[3] == "true";
        def.freshOnly = parts[4] == "true";
        m_addons.push_back(def);
    }
    return true;
}