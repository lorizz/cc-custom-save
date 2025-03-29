#pragma once
#include <vector>
#include <string>
#include "character_data.h"

extern std::vector<CharacterData> addonCharacters;

std::vector<CharacterData> readAddonCharacters(const std::string& iniFilename);

void InitializeAddonCharacters(const std::string& iniFilename);
