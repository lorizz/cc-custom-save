#pragma once

#include <cstdint>
#include <array>
#include <json.hpp>

#pragma pack(push, 1)
struct GlobalData {
    uint8_t vibration;
    uint8_t voice_volume;
    uint8_t music_volume;
    uint8_t sfx_volume;
    uint8_t gore;
    std::array<uint8_t, 2> pad1;
    uint8_t arenas_bitflag;
    std::array<uint8_t, 4> animals_bitflags;
    std::array<uint8_t, 16> items_bitflags;
    std::array<uint8_t, 8> items_expansion_bitflags;
    std::array<uint8_t, 12> pad2;
    uint32_t back_off_barbarian_time;
    uint8_t back_off_barbarian_character;
    std::array<uint8_t, 11> pad3;
};
#pragma pack(pop)

namespace nlohmann {
    template <>
    struct adl_serializer<GlobalData> {
        static void to_json(json& j, const GlobalData& g) {
            j = {
                {"vibration", g.vibration}, {"voice_volume", g.voice_volume},
                {"music_volume", g.music_volume}, {"sfx_volume", g.sfx_volume},
                {"gore", g.gore}, {"arenas_bitflag", g.arenas_bitflag},
                {"animals_bitflags", g.animals_bitflags}, {"items_bitflags", g.items_bitflags},
                {"items_expansion_bitflags", g.items_expansion_bitflags},
                {"back_off_barbarian_time", g.back_off_barbarian_time},
                {"back_off_barbarian_character", g.back_off_barbarian_character}
            };
        }
        static void from_json(const json& j, GlobalData& g) {
            g = {};
            g.vibration = j.value("vibration", (uint8_t)1);
            g.voice_volume = j.value("voice_volume", (uint8_t)100);
            g.music_volume = j.value("music_volume", (uint8_t)75);
            g.sfx_volume = j.value("sfx_volume", (uint8_t)80);
            g.gore = j.value("gore", (uint8_t)1);
            g.arenas_bitflag = j.value("arenas_bitflag", (uint8_t)0);
            if (j.contains("animals_bitflags")) j.at("animals_bitflags").get_to(g.animals_bitflags);
            if (j.contains("items_bitflags")) j.at("items_bitflags").get_to(g.items_bitflags);
            if (j.contains("items_expansion_bitflags")) j.at("items_expansion_bitflags").get_to(g.items_expansion_bitflags);
            g.back_off_barbarian_time = j.value("back_off_barbarian_time", 0u);
            g.back_off_barbarian_character = j.value("back_off_barbarian_character", (uint8_t)0);
        }
    };
}