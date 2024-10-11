#pragma once

#include <iostream>
#include <cstdint>
#include <json.hpp>
#include "logger.h"

#include <array>

// Function to inject code
void InjectCode(uintptr_t base);

// Struct to represent the first 64 bytes of the save file
struct GlobalUnlocks {
    uint8_t unknownByte;
    uint8_t voiceVolume;
    uint8_t musicVolume;
    uint8_t sfxVolume;
    std::array<uint8_t, 4> unknownArray1;
    uint8_t arenas;
    std::array<uint8_t, 4> animals;
    std::array<uint8_t, 4> itemUnlocks;
    uint8_t relics;
    std::array<uint8_t, 12> weapons;
    std::array<uint8_t, 8> dlcWeapons;
    std::array<uint8_t, 10> unknownArray2;
    std::array<uint8_t, 4> backOffBarbarianTime;
    uint8_t backOffBarbarianBestChar;
    std::array<uint8_t, 11> unknownArray3;
};

// Struct to represent 48 bytes of character data
struct CharacterData {
    uint8_t unlockStatus;
    uint8_t level;
    std::array<uint8_t, 4> xp;
    uint8_t weapon;
    uint8_t pet;
    std::array<uint8_t, 4> stats;
    std::array<uint8_t, 3> normalLevelUnlocks;
    std::array<uint8_t, 3> consumableItems;
    uint8_t nonConsumableItems;
    std::array<uint8_t, 4> money;
    uint8_t insaneModeStoreUnlock;
    std::array<uint8_t, 3> insaneLevelUnlocks;
    uint8_t skullStatus;
    uint8_t dugUpItems;
    uint8_t princessKisses;
    std::array<uint8_t, 2> arenaWins;
    std::array<uint8_t, 16> unknown;
};
