#pragma once
#include <string>
#include <vector>
#include <cstdint>

static constexpr int BASE_CHAR_COUNT = 32;
static constexpr int WORKSHOP_CHAR_COUNT = 10;
static constexpr int TOTAL_ORIGINAL_CHARS = BASE_CHAR_COUNT + WORKSHOP_CHAR_COUNT; // 42 = 0x2A

struct AddonCharacterDef {
    std::string id;
    uint8_t weapon;
    uint8_t animal;
    bool unlocked;
    bool freshOnly;
};

class AddonCharacterConfig {
public:
    static AddonCharacterConfig& getInstance();
    bool loadFromIni(const std::string& iniPath);

    const std::vector<AddonCharacterDef>& getAddons() const { return m_addons; }
    int getAddonCount() const { return (int)m_addons.size(); }

private:
    std::vector<AddonCharacterDef> m_addons;
};