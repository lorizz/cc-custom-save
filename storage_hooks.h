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