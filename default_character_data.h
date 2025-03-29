#pragma once
#include <vector>
#include "character_data.h"

// Declaration of the global default characters vector.
extern std::vector<CharacterData> defaultCharacters;

// Declaration of a function to initialize the default characters.
std::vector<CharacterData> InitializeDefaultCharacters();
