#pragma once

#include <cstdint>
#include <array>
#include <json.hpp>

#pragma pack(push, 1)
struct CharacterData {
    uint8_t unlocked;
    uint8_t level;
    std::array<uint8_t, 4> experience;
    uint8_t weapon;
    uint8_t animal;
    uint8_t strength;
    uint8_t defense;
    uint8_t magic;
    uint8_t agility;
    std::array<uint8_t, 3> normal_levels_bitflags;
    uint8_t potions;
    uint8_t bombs;
    uint8_t beefies;
    uint8_t relics_bitflag;
    std::array<uint8_t, 4> gold;
    uint8_t insane_mode;
    std::array<uint8_t, 3> insane_levels_bitflags;
    uint8_t skull;
    uint8_t shovel_items;
    uint8_t princess_kisses;
    std::array<uint16_t, 4> arena_wins;
    uint32_t arena_losses;
    uint32_t arena_points;
    std::array<uint8_t, 2> pad1;
};
#pragma pack(pop)

namespace nlohmann {
    template <>
    struct adl_serializer<CharacterData> {
        static void to_json(json& j, const CharacterData& c) {
            j = {
                {"unlocked", (c.unlocked == 0x80)}, {"level", c.level},
                {"experience", c.experience}, {"weapon", c.weapon},
                {"animal", c.animal}, {"strength", c.strength},
                {"defense", c.defense}, {"magic", c.magic},
                {"agility", c.agility}, {"normal_levels_bitflags", c.normal_levels_bitflags},
                {"potions", c.potions}, {"bombs", c.bombs},
                {"beefies", c.beefies}, {"relics_bitflag", c.relics_bitflag},
                {"gold", c.gold}, {"insane_mode", c.insane_mode},
                {"insane_levels_bitflags", c.insane_levels_bitflags},
                {"skull", c.skull}, {"shovel_items", c.shovel_items},
                {"princess_kisses", c.princess_kisses}, {"arena_wins", c.arena_wins},
                {"arena_losses", c.arena_losses}, {"arena_points", c.arena_points}
            };
        }
        static void from_json(const json& j, CharacterData& c) {
            c = {};
            c.unlocked = j.value("unlocked", false) ? 0x80 : 0x00;
            c.level = j.value("level", (uint8_t)0);
            if (j.contains("experience")) j.at("experience").get_to(c.experience);
            c.weapon = j.value("weapon", (uint8_t)0);
            c.animal = j.value("animal", (uint8_t)0);
            c.strength = j.value("strength", (uint8_t)1);
            c.defense = j.value("defense", (uint8_t)1);
            c.magic = j.value("magic", (uint8_t)1);
            c.agility = j.value("agility", (uint8_t)1);
            if (j.contains("normal_levels_bitflags")) j.at("normal_levels_bitflags").get_to(c.normal_levels_bitflags);
            c.potions = j.value("potions", (uint8_t)0);
            c.bombs = j.value("bombs", (uint8_t)0);
            c.beefies = j.value("beefies", (uint8_t)0);
            c.relics_bitflag = j.value("relics_bitflag", (uint8_t)0);
            if (j.contains("gold")) j.at("gold").get_to(c.gold);
            c.insane_mode = j.value("insane_mode", (uint8_t)0);
            if (j.contains("insane_levels_bitflags")) j.at("insane_levels_bitflags").get_to(c.insane_levels_bitflags);
            c.skull = j.value("skull", (uint8_t)0);
            c.shovel_items = j.value("shovel_items", (uint8_t)0);
            c.princess_kisses = j.value("princess_kisses", (uint8_t)0);
            if (j.contains("arena_wins")) j.at("arena_wins").get_to(c.arena_wins);
            c.arena_losses = j.value("arena_losses", 0u);
            c.arena_points = j.value("arena_points", 0u);
        }
    };
}