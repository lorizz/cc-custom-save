#pragma once

#include <string>
#include <vector>
#include <cstdint>

// ============================================================
//  Save file layout constants
// ============================================================
static constexpr int GLOBAL_UNLOCK_SIZE = 64;
static constexpr int CHAR_DATA_SIZE = 48;

// ============================================================
//  Character counts
// ============================================================
static constexpr int BASE_CLASSIC_COUNT = 31;
static constexpr int BASE_FRESH_COUNT = 32;
static constexpr int WORKSHOP_CHAR_COUNT = 10;
static constexpr int STREAMABLE_CHAR_COUNT = 72;
static constexpr int BASE_CHAR_COUNT = BASE_FRESH_COUNT;
static constexpr int TOTAL_ORIGINAL_CHARS = BASE_CHAR_COUNT + WORKSHOP_CHAR_COUNT;

// ============================================================
//  Save buffer layout (base, no addons)
//   [0  .. 63 ]  global unlock flags  (GLOBAL_UNLOCKS_SIZE)
//   [64 .. 2079]  42 chars * 48 bytes  (TOTAL_ORIGINAL_CHARS * CHAR_DATA_SIZE)
//   [2080..2127]  keybinds             (CHAR_DATA_SIZE)
//   total = 2128 = 0x850
// ============================================================
static constexpr int BASE_SAVE_SIZE = 0x850;
static constexpr int BASE_SAVE_CAPACITY = 0x84C; // size - 4 checksum
static constexpr int BASE_KEYBINDS_OFFSET = 0x820;
static constexpr int BASE_KEYBINDS_CHECK = 0x832; // keybinds offset + 18

// ============================================================
//  Character table
// ============================================================
static constexpr int TOTAL_ORIGINAL_SLOTS = 73;
static constexpr int CHAR_TABLE_ENTRY_SIZE = 32;
static constexpr int CHAR_TABLE_BASE_SIZE = 14728; // malloc for d_characterTable

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