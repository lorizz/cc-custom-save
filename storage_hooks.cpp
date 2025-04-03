#include <fstream>
#include <iostream>
#include <vector>
#include <json.hpp>
#include <sstream>
#include <map>
#include <string>
#include <stdexcept>
#include <iomanip>
#include "storage_hooks.h"
#include "character_data.h"
#include "default_character_data.h"
#include "dynamic_character_data.h"
#include "addon_characters.h"

using json = nlohmann::json;

extern std::vector<FieldInfo> gCharacterStructure;

const std::string GLOBAL_UNLOCKS_FILENAME = "scripts/saves/global_unlocks.json";
const std::string CHARACTERS_DIR = "scripts/saves/characters/";
const std::string ADDON_CHARACTERS_DIR = "scripts/saves/characters/addon/";

// Function to convert GlobalUnlocks to JSON
json globalUnlocksToJson(const GlobalUnlocks& unlocks) {
	return json{
		{"unknownByte", unlocks.unknownByte},
		{"voiceVolume", unlocks.voiceVolume},
		{"musicVolume", unlocks.musicVolume},
		{"sfxVolume", unlocks.sfxVolume},
		{"unknownArray1", unlocks.unknownArray1},
		{"arenas", unlocks.arenas},
		{"animals", unlocks.animals},
		{"itemUnlocks", unlocks.itemUnlocks},
		{"relics", unlocks.relics},
		{"weapons", unlocks.weapons},
		{"dlcWeapons", unlocks.dlcWeapons},
		{"unknownArray2", unlocks.unknownArray2},
		{"backOffBarbarianTime", unlocks.backOffBarbarianTime},
		{"backOffBarbarianBestChar", unlocks.backOffBarbarianBestChar},
		{"unknownArray3", unlocks.unknownArray3}
	};
}

// Function to convert JSON to GlobalUnlocks
GlobalUnlocks jsonToGlobalUnlocks(const json& j) {
	GlobalUnlocks unlocks;
	unlocks.unknownByte = j["unknownByte"];
	unlocks.voiceVolume = j["voiceVolume"];
	unlocks.musicVolume = j["musicVolume"];
	unlocks.sfxVolume = j["sfxVolume"];
	unlocks.unknownArray1 = j["unknownArray1"].get<std::array<uint8_t, 4>>();
	unlocks.arenas = j["arenas"];
	unlocks.animals = j["animals"].get<std::array<uint8_t, 4>>();
	unlocks.itemUnlocks = j["itemUnlocks"].get<std::array<uint8_t, 4>>();
	unlocks.relics = j["relics"];
	unlocks.weapons = j["weapons"].get<std::array<uint8_t, 12>>();
	unlocks.dlcWeapons = j["dlcWeapons"].get<std::array<uint8_t, 8>>();
	unlocks.unknownArray2 = j["unknownArray2"].get<std::array<uint8_t, 10>>();
	unlocks.backOffBarbarianTime = j["backOffBarbarianTime"].get<std::array<uint8_t, 4>>();
	unlocks.backOffBarbarianBestChar = j["backOffBarbarianBestChar"];
	unlocks.unknownArray3 = j["unknownArray3"].get<std::array<uint8_t, 11>>();
	return unlocks;
}

// Function to convert CharacterData to JSON
json characterDataToJson(const CharacterData& character) {
	json j;

	for (const auto& item : character.getData()) {
		const std::string& fieldName = item.first;
		const std::vector<uint8_t> fieldValue = item.second;
		std::vector<int> intValues(fieldValue.begin(), fieldValue.end());
		j[fieldName] = intValues;
	}

	j["id"] = character.getId();

	return j;
}

// Function to convert JSON to CharacterData
CharacterData jsonToCharacterData(const json& j, const std::map<std::string, size_t>& memoryStructure) {
	std::string characterId = j.contains("id") ? j["id"].get<std::string>() : "unknown"; // Default value if not present
	uint8_t weapon = j.contains("weapon") ? j["weapon"].get<uint8_t>() : 0; // Default weapon if not present

	CharacterData characterData(characterId, weapon);

	// Populate CharacterData using the JSON keys and values dynamically
	for (const auto& data : memoryStructure) {
		if (j.contains(data.first)) {
			std::vector<uint8_t> vecValue = j[data.first].get<std::vector<uint8_t>>();
			characterData.addData(data.first, vecValue);
		}
		else if (j[data.first].is_number_integer()) {
			characterData.addData(data.first, { j[data.first].get<uint8_t>() });
		}
	}

	return characterData;
}

GlobalUnlocks InitializeGlobalUnlocks() {
	GlobalUnlocks initGlobalUnlocks;

	initGlobalUnlocks.unknownByte = 0x00;
	initGlobalUnlocks.voiceVolume = 0x05;
	initGlobalUnlocks.musicVolume = 0x05;
	initGlobalUnlocks.sfxVolume = 0x05;
	initGlobalUnlocks.unknownArray1 = { 0x00, 0x00, 0x00, 0x00 };
	initGlobalUnlocks.arenas = 0x00;
	initGlobalUnlocks.animals = { 0x00, 0x00, 0x00, 0x00 };
	initGlobalUnlocks.itemUnlocks = { 0x00, 0x00, 0x00, 0x00 };
	initGlobalUnlocks.relics = 0x00;
	initGlobalUnlocks.weapons = { 0x20, 0x00, 0x00, 0x08, 0x00, 0x02, 0x00, 0x05, 0x02, 0xF6, 0xAF, 0x00 };
	initGlobalUnlocks.dlcWeapons = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	initGlobalUnlocks.unknownArray2 = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	initGlobalUnlocks.backOffBarbarianTime = { 0x00, 0x00, 0x00, 0x00 };
	initGlobalUnlocks.backOffBarbarianBestChar = 0x00;
	initGlobalUnlocks.unknownArray3 = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	return initGlobalUnlocks;
}

void InitializeJsonFile() {
	Logger& l = Logger::Instance();

	// Initialize global unlocks file
	if (!std::ifstream(GLOBAL_UNLOCKS_FILENAME).good()) {
		json globalUnlocksData = globalUnlocksToJson(InitializeGlobalUnlocks());
		std::ofstream globalUnlocksFile(GLOBAL_UNLOCKS_FILENAME);
		globalUnlocksFile << globalUnlocksData.dump() << std::endl;
		globalUnlocksFile.close();
	}

	if (gCharacterStructure.empty()) {
		return;
	}

	for (const auto& character : defaultCharacters) {
		std::string characterId = character.getId();
		std::string filename = CHARACTERS_DIR + characterId + ".json";

		std::ifstream testFile(filename);
		if (testFile.good()) {
			json existingJson;
			testFile >> existingJson;
			testFile.close();
		}
		else {
			json j = characterDataToJson(character);

			std::ofstream outFile(filename);
			outFile << j.dump() << std::endl;
			outFile.close();
		}
	}

	for (const auto& character : addonCharacters) {
		std::string characterId = character.getId();
		std::string filename = ADDON_CHARACTERS_DIR + characterId + ".json";

		std::ifstream testFile(filename);
		if (testFile.good()) {
			json existingJson;
			testFile >> existingJson;
			testFile.close();
		}
		else {
			json j = characterDataToJson(character);

			std::ofstream outFile(filename);
			outFile << j.dump() << std::endl;
			outFile.close();
		}
	}
}


void WriteByteToJson(uint32_t offset, uint8_t byteValue) {
	Logger& l = Logger::Instance();
	l.Get()->info("Writing offset {}", offset);
	l.Get()->flush();

	if (offset < 64) {
		std::ifstream inputFile(GLOBAL_UNLOCKS_FILENAME);
		json globalUnlocksData;
		inputFile >> globalUnlocksData;
		inputFile.close();

		GlobalUnlocks globalUnlocks = jsonToGlobalUnlocks(globalUnlocksData);
		if (offset == 0) globalUnlocks.unknownByte = byteValue;
		else if (offset == 1) globalUnlocks.voiceVolume = byteValue;
		else if (offset == 2) globalUnlocks.musicVolume = byteValue;
		else if (offset == 3) globalUnlocks.sfxVolume = byteValue;
		else if (offset >= 4 && offset <= 7) globalUnlocks.unknownArray1[offset - 4] = byteValue;
		else if (offset == 8) globalUnlocks.arenas = byteValue;
		else if (offset >= 9 && offset <= 12) globalUnlocks.animals[offset - 9] = byteValue;
		else if (offset >= 13 && offset <= 16) globalUnlocks.itemUnlocks[offset - 13] = byteValue;
		else if (offset == 17) globalUnlocks.relics = byteValue;
		else if (offset >= 18 && offset <= 29) globalUnlocks.weapons[offset - 18] = byteValue;
		else if (offset >= 30 && offset <= 37) globalUnlocks.dlcWeapons[offset - 30] = byteValue;
		else if (offset >= 38 && offset <= 47) globalUnlocks.unknownArray2[offset - 38] = byteValue;
		else if (offset >= 48 && offset <= 51) globalUnlocks.backOffBarbarianTime[offset - 48] = byteValue;
		else if (offset == 52) globalUnlocks.backOffBarbarianBestChar = byteValue;
		else if (offset >= 53 && offset <= 63) globalUnlocks.unknownArray3[offset - 53] = byteValue;

		std::ofstream outputFile(GLOBAL_UNLOCKS_FILENAME);
		outputFile << globalUnlocksToJson(globalUnlocks).dump() << std::endl;
		outputFile.close();
	}
	else {
		uint32_t totalBytes = getTotalCharacterDataBytes(gCharacterStructure);
		uint32_t totalCharactersBytes = (defaultCharacters.size() + addonCharacters.size()) * totalBytes;
		if (offset < 64 + totalCharactersBytes) {
			uint32_t characterOffset = offset - 64;
			size_t defaultTotalBytes = defaultCharacters.size() * totalBytes;
			if (characterOffset < defaultTotalBytes) {
				size_t characterIndex = characterOffset / totalBytes;
				size_t fieldAbsoluteOffset = characterOffset % totalBytes;

				std::string characterId = defaultCharacters[characterIndex].getId();
				std::string filename = CHARACTERS_DIR + characterId + ".json";

				std::ifstream inputFile(filename);
				if (!inputFile.is_open()) {
					throw std::runtime_error("Unable to open file: " + filename);
				}
				json charData;
				inputFile >> charData;
				inputFile.close();

				std::string fieldName;
				size_t internalIndex;
				if (getFieldInfoFromOffset(gCharacterStructure, fieldAbsoluteOffset, fieldName, internalIndex)) {
					if (charData[fieldName].is_array()) {
						charData[fieldName][internalIndex] = byteValue;
					}
					else {
						charData[fieldName] = byteValue;
					}
				}
				else {
					throw std::runtime_error("Offset out of CharacterData bounds");
				}

				std::ofstream outputFile(filename);
				outputFile << charData.dump() << std::endl;
				outputFile.close();
			}
			else {
				// Addon characters (invariato)
				uint32_t addonOffset = characterOffset - defaultTotalBytes;
				size_t addonTotalBytes = addonCharacters.size() * totalBytes;
				if (addonOffset >= addonTotalBytes) {
					throw std::runtime_error("Offset out of bounds for both default and addon characters");
				}
				size_t characterIndex = addonOffset / totalBytes;
				size_t fieldAbsoluteOffset = addonOffset % totalBytes;

				std::string characterId = addonCharacters[characterIndex].getId();
				std::string filename = ADDON_CHARACTERS_DIR + characterId + ".json";

				std::ifstream inputFile(filename);
				if (!inputFile.is_open()) {
					throw std::runtime_error("Unable to open file: " + filename);
				}
				json charData;
				inputFile >> charData;
				inputFile.close();

				std::string fieldName;
				size_t internalIndex;
				if (getFieldInfoFromOffset(gCharacterStructure, fieldAbsoluteOffset, fieldName, internalIndex)) {
					if (charData[fieldName].is_array()) {
						charData[fieldName][internalIndex] = byteValue;
					}
					else {
						charData[fieldName] = byteValue;
					}
				}
				else {
					throw std::runtime_error("Offset out of CharacterData bounds for addon character");
				}

				std::ofstream outputFile(filename);
				outputFile << charData.dump() << std::endl;
				outputFile.close();
			}
		}
	}
}

uint8_t ReadByteFromJson(uint32_t offset) {
	Logger& l = Logger::Instance();

	uint8_t byteValue = 0;
	l.Get()->info("Reading offset {}", offset);
	l.Get()->flush();
	if (offset < 64) {
		// Read from GlobalUnlocks
		std::ifstream inputFile(GLOBAL_UNLOCKS_FILENAME);
		json globalUnlocksData;
		inputFile >> globalUnlocksData;
		inputFile.close();

		GlobalUnlocks globalUnlocks = jsonToGlobalUnlocks(globalUnlocksData);
		if (offset == 0) byteValue = globalUnlocks.unknownByte;
		else if (offset == 1) byteValue = globalUnlocks.voiceVolume;
		else if (offset == 2) byteValue = globalUnlocks.musicVolume;
		else if (offset == 3) byteValue = globalUnlocks.sfxVolume;
		else if (offset >= 4 && offset <= 7) byteValue = globalUnlocks.unknownArray1[offset - 4];
		else if (offset == 8) byteValue = globalUnlocks.arenas;
		else if (offset >= 9 && offset <= 12) byteValue = globalUnlocks.animals[offset - 9];
		else if (offset >= 13 && offset <= 16) byteValue = globalUnlocks.itemUnlocks[offset - 13];
		else if (offset == 17) byteValue = globalUnlocks.relics;
		else if (offset >= 18 && offset <= 29) byteValue = globalUnlocks.weapons[offset - 18];
		else if (offset >= 30 && offset <= 37) byteValue = globalUnlocks.dlcWeapons[offset - 30];
		else if (offset >= 38 && offset <= 47) byteValue = globalUnlocks.unknownArray2[offset - 38];
		else if (offset >= 48 && offset <= 51) byteValue = globalUnlocks.backOffBarbarianTime[offset - 48];
		else if (offset == 52) byteValue = globalUnlocks.backOffBarbarianBestChar;
		else if (offset >= 53 && offset <= 63) byteValue = globalUnlocks.unknownArray3[offset - 53];
	}
	else {
		uint32_t characterOffset = offset - 64;
		size_t totalBytes = getTotalCharacterDataBytes(gCharacterStructure);

		size_t defaultTotalBytes = defaultCharacters.size() * totalBytes;
		if (characterOffset < defaultTotalBytes) {
			size_t characterIndex = characterOffset / totalBytes;
			size_t fieldAbsoluteOffset = characterOffset % totalBytes;

			std::string characterId = defaultCharacters[characterIndex].getId();
			std::string filename = CHARACTERS_DIR + characterId + ".json";

			std::ifstream inputFile(filename);
			if (!inputFile.is_open()) {
				throw std::runtime_error("Unable to open file: " + filename);
			}
			json charData;
			inputFile >> charData;
			inputFile.close();

			std::string fieldName;
			size_t internalIndex;
			if (getFieldInfoFromOffset(gCharacterStructure, fieldAbsoluteOffset, fieldName, internalIndex)) {
				if (charData[fieldName].is_array()) {
					byteValue = charData[fieldName][internalIndex];
				}
				else {
					byteValue = charData[fieldName];
				}
			}
			else {
				throw std::runtime_error("Offset out of CharacterData bounds for default character");
			}
		}
		else {
			uint32_t addonOffset = characterOffset - defaultTotalBytes;
			size_t addonTotalBytes = addonCharacters.size() * totalBytes;
			if (addonOffset >= addonTotalBytes) {
				throw std::runtime_error("Offset out of bounds for both default and addon characters");
			}
			size_t characterIndex = addonOffset / totalBytes;
			size_t fieldAbsoluteOffset = addonOffset % totalBytes;

			std::string characterId = addonCharacters[characterIndex].getId();
			std::string filename = ADDON_CHARACTERS_DIR + characterId + ".json";

			std::ifstream inputFile(filename);
			if (!inputFile.is_open()) {
				throw std::runtime_error("Unable to open file: " + filename);
			}
			json charData;
			inputFile >> charData;
			inputFile.close();

			std::string fieldName;
			size_t internalIndex;
			if (getFieldInfoFromOffset(gCharacterStructure, fieldAbsoluteOffset, fieldName, internalIndex)) {
				if (charData[fieldName].is_array()) {
					byteValue = charData[fieldName][internalIndex];
				}
				else {
					byteValue = charData[fieldName];
				}
			}
			else {
				throw std::runtime_error("Offset out of CharacterData bounds for addon character");
			}
		}
	}
	return byteValue;
}

// The assembly code injection functions remain the same
__declspec(naked) void WriteInjectedCode() {
	__asm {
		cmp esi, 0x99999
		jb jb_label
		mov eax, dword ptr ss : [ebp + 8]
		pop esi
		mov dword ptr ds : [eax] , 1
		mov dword ptr ds : [eax + 4] , 0
		pop ebp
		ret 0x0C
		jb_label :
		mov ecx, dword ptr ds : [edx + 4]
		mov al, byte ptr ss : [ebp + 0x10]
		mov byte ptr ds : [esi + ecx] , al
		push edx
		push ecx
		push eax
		push esi

		call WriteByteToJson

		pop esi
		pop eax
		pop ecx
		pop edx

		mov eax, dword ptr ss : [ebp + 8]
		inc dword ptr ds : [edx + 0x0C]
		pop esi
		mov dword ptr ds : [eax] , 0
		mov dword ptr ds : [eax + 4] , 0
		pop ebp
		ret 0x0C
	}
}

__declspec(naked) void ReadInjectedCode() {
	__asm {
		je je_label
		mov ecx, dword ptr ds : [edx + 0xC]
		cmp ecx, 0x99999 // TODO here 0x99999
		jb jb_label
		mov eax, dword ptr ds : [edx + 0xC]
		mov byte ptr ds : [eax] , 0
		je_label :
		mov eax, dword ptr ss : [ebp + 8]
		mov dword ptr ds : [eax] , 1
		mov dword ptr ds : [eax + 4] , 0
		pop ebp
		ret 0x0C
		jb_label :
		mov eax, dword ptr ds : [edx + 4]

		push ebx
		push eax
		push edx
		push esi
		push ecx // Offset

		call ReadByteFromJson
		mov bl, al

		pop ecx
		pop esi
		pop edx
		pop eax

		mov eax, dword ptr ss : [ebp + 0x10]
		mov byte ptr ds : [eax] , bl
		pop ebx
		mov eax, dword ptr ss : [ebp + 8]
		inc dword ptr ds : [edx + 0x0C]
		mov dword ptr ds : [eax] , 0
		mov dword ptr ds : [eax + 4] , 0
		pop ebp
		ret 0x0C
	}
}

__declspec(naked) void UnlockInjectedCode() {
	__asm {
		mov dword ptr ds : [eax + 0x0C] , ecx
		mov eax, dword ptr ss : [ebp + 8]
		mov dword ptr ds : [eax] , 0
		mov dword ptr ds : [eax + 4] , 0
		pop ebp
		ret 0x0C
	}
}

__declspec(naked) void OriginalWriteStorageCloned()
{
	__asm
	{
		push ebp
		mov ebp, esp
		mov ecx, dword ptr ss : [ebp + 0xC]
		mov eax, dword ptr ds : [0xAA8598] // TODO: This must be copied from (base + 0x098CEC)
		mov edx, dword ptr ds : [eax + ecx * 4 + 0x14]
		cmp byte ptr ds : [edx], 0
		jne label_87AEC8
		mov eax, dword ptr ss : [ebp + 8]
		mov dword ptr ds : [eax], 1
		mov dword ptr ds : [eax + 4], 0
		pop ebp
		ret 0xC

		label_87AEC8 :
			push esi
			mov esi, dword ptr ds : [edx + 0xC]
			cmp esi, dword ptr ds : [edx + 8]
			jb label_87AEE6
			mov eax, dword ptr ss : [ebp + 8]
			pop esi
			mov dword ptr ds : [eax], 1
			mov dword ptr ds : [eax + 4], 0
			pop ebp
			ret 0xC

		label_87AEE6 :
			mov ecx, dword ptr ds : [edx + 4]
			mov al, byte ptr ss : [ebp + 0x10]
			mov byte ptr ds : [esi + ecx], al
			mov eax, dword ptr ss : [ebp + 8]
			inc dword ptr ds : [edx + 0xC]
			pop esi
			mov dword ptr ds : [eax], 0
			mov dword ptr ds : [eax + 4], 0
			pop ebp
			ret 0xC
	}
}

BYTE* InjectStorageCode(uintptr_t base) {
	BYTE* newMemory = (BYTE*)VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (newMemory == NULL) {
		return 0x0;
	}

	// Set up our code cave
	BYTE* codeCave = newMemory;
	// Write our assembly instructions as bytes
	int offset = 0;

	// push ebp
	codeCave[offset++] = 0x55;

	// mov ebp, esp
	codeCave[offset++] = 0x89;
	codeCave[offset++] = 0xE5;

	// mov ecx, dword ptr ss:[ebp + 0xC]
	codeCave[offset++] = 0x8B;
	codeCave[offset++] = 0x4D;
	codeCave[offset++] = 0x0C;

	// mov eax, dword ptr ds:[0xAA8598]
	DWORD addressValue = *(DWORD*)(base + 0x098CC8);
	*(DWORD*)(codeCave + offset) = addressValue;
	offset += 5;

	// mov edx, dword ptr ds:[eax + ecx * 4 + 0x14]
	codeCave[offset++] = 0x8B;
	codeCave[offset++] = 0x54;
	codeCave[offset++] = 0x88;
	codeCave[offset++] = 0x14;

	// cmp byte ptr ds:[edx], 0
	codeCave[offset++] = 0x80;
	codeCave[offset++] = 0x3A;
	codeCave[offset++] = 0x00;

	// jne label_87AEC8
	codeCave[offset++] = 0x75;
	codeCave[offset++] = 0x14; // Salto relativo che dovrà essere aggiustato

	// mov eax, dword ptr ss:[ebp + 8]
	codeCave[offset++] = 0x8B;
	codeCave[offset++] = 0x45;
	codeCave[offset++] = 0x08;

	// mov dword ptr ds:[eax], 1
	codeCave[offset++] = 0xC7;
	codeCave[offset++] = 0x00;
	*(DWORD*)(codeCave + offset) = 0x01;
	offset += 4;

	// mov dword ptr ds:[eax + 4], 0
	codeCave[offset++] = 0xC7;
	codeCave[offset++] = 0x40;
	codeCave[offset++] = 0x04;
	*(DWORD*)(codeCave + offset) = 0x00;
	offset += 4;

	// pop ebp
	codeCave[offset++] = 0x5D;

	// ret 0xC
	codeCave[offset++] = 0xC2;
	codeCave[offset++] = 0x0C;
	codeCave[offset++] = 0x00;

	// label_87AEC8:
	// push esi
	codeCave[offset++] = 0x56;

	// mov esi, dword ptr ds:[edx + 0xC]
	codeCave[offset++] = 0x8B;
	codeCave[offset++] = 0x72;
	codeCave[offset++] = 0x0C;

	// cmp esi, dword ptr ds:[edx + 8]
	codeCave[offset++] = 0x3B;
	codeCave[offset++] = 0x72;
	codeCave[offset++] = 0x08;

	// jb label_87AEE6
	codeCave[offset++] = 0x72;
	codeCave[offset++] = 0x15; // Salto relativo che dovrà essere aggiustato

	// mov eax, dword ptr ss:[ebp + 8]
	codeCave[offset++] = 0x8B;
	codeCave[offset++] = 0x45;
	codeCave[offset++] = 0x08;

	// pop esi
	codeCave[offset++] = 0x5E;

	// mov dword ptr ds:[eax], 1
	codeCave[offset++] = 0xC7;
	codeCave[offset++] = 0x00;
	*(DWORD*)(codeCave + offset) = 0x01;
	offset += 4;

	// mov dword ptr ds:[eax + 4], 0
	codeCave[offset++] = 0xC7;
	codeCave[offset++] = 0x40;
	codeCave[offset++] = 0x04;
	*(DWORD*)(codeCave + offset) = 0x00;
	offset += 4;

	// pop ebp
	codeCave[offset++] = 0x5D;

	// ret 0xC
	codeCave[offset++] = 0xC2;
	codeCave[offset++] = 0x0C;
	codeCave[offset++] = 0x00;

	// label_87AEE6:
	// mov ecx, dword ptr ds:[edx + 4]
	codeCave[offset++] = 0x8B;
	codeCave[offset++] = 0x4A;
	codeCave[offset++] = 0x04;

	// mov al, byte ptr ss:[ebp + 0x10]
	codeCave[offset++] = 0x8A;
	codeCave[offset++] = 0x45;
	codeCave[offset++] = 0x10;

	// mov byte ptr ds:[esi + ecx], al
	codeCave[offset++] = 0x88;
	codeCave[offset++] = 0x04;
	codeCave[offset++] = 0x0E;

	// mov eax, dword ptr ss:[ebp + 8]
	codeCave[offset++] = 0x8B;
	codeCave[offset++] = 0x45;
	codeCave[offset++] = 0x08;

	// inc dword ptr ds:[edx + 0xC]
	codeCave[offset++] = 0xFF;
	codeCave[offset++] = 0x42;
	codeCave[offset++] = 0x0C;

	// pop esi
	codeCave[offset++] = 0x5E;

	// mov dword ptr ds:[eax], 0
	codeCave[offset++] = 0xC7;
	codeCave[offset++] = 0x00;
	*(DWORD*)(codeCave + offset) = 0x00;
	offset += 4;

	// mov dword ptr ds:[eax + 4], 0
	codeCave[offset++] = 0xC7;
	codeCave[offset++] = 0x40;
	codeCave[offset++] = 0x04;
	*(DWORD*)(codeCave + offset) = 0x00;
	offset += 4;

	// pop ebp
	codeCave[offset++] = 0x5D;

	// ret 0xC
	codeCave[offset++] = 0xC2;
	codeCave[offset++] = 0x0C;
	codeCave[offset++] = 0x00;

	return codeCave;
}

void InjectCode(uintptr_t base) {
	Logger& l = Logger::Instance();
	InitializeJsonFile();

	DWORD oldProtect;
	unsigned char* writeTargetAddress = reinterpret_cast<unsigned char*>(base + 0x4AECC);
	VirtualProtect(writeTargetAddress, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
	*writeTargetAddress = 0xE9; // JMP opcode
	*reinterpret_cast<unsigned int*>(writeTargetAddress + 1) = (reinterpret_cast<unsigned char*>(&WriteInjectedCode) - writeTargetAddress - 5);
	VirtualProtect(writeTargetAddress, 5, oldProtect, &oldProtect);

	unsigned char* readTargetAddress = reinterpret_cast<unsigned char*>(base + 0x4AE4D);
	VirtualProtect(readTargetAddress, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
	*readTargetAddress = 0xE9;
	*reinterpret_cast<unsigned int*>(readTargetAddress + 1) = (reinterpret_cast<unsigned char*>(&ReadInjectedCode) - readTargetAddress - 5);
	VirtualProtect(readTargetAddress, 5, oldProtect, &oldProtect);

	unsigned char* unlockTargetAddress = reinterpret_cast<unsigned char*>(base + 0x4AF39);
	VirtualProtect(unlockTargetAddress, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
	*unlockTargetAddress = 0xE9;
	*reinterpret_cast<unsigned int*>(unlockTargetAddress + 1) = (reinterpret_cast<unsigned char*>(&UnlockInjectedCode) - unlockTargetAddress - 5);
	VirtualProtect(unlockTargetAddress, 5, oldProtect, &oldProtect);

	auto storageCodeOffset = InjectStorageCode(base);

	std::vector<uintptr_t> keybindOffsets = {
		0x098CFA,
		0x098D13,
		0x098D2C,
		0x098D45,
		0x098D5E,
		0x098D77,
		0x098D90,
		0x098DA9,
		0x098DC2,
		0x098DDB,
		0x098DF4,
		0x098E0D,
		0x098E26,
		0x098E33,
		0x098E40,
		0x098E4D,
		0x098E66,
		0x098E82
	};

	for (uintptr_t off : keybindOffsets) {
		unsigned char* keybindTargetAddress = reinterpret_cast<unsigned char*>(base + off);
		DWORD oldProtect;
		VirtualProtect(keybindTargetAddress, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

		*keybindTargetAddress = 0xE8; // Opcode per CALL
		uintptr_t relativeAddress = reinterpret_cast<uintptr_t>(storageCodeOffset) - reinterpret_cast<uintptr_t>(keybindTargetAddress) - 5;
		*reinterpret_cast<uintptr_t*>(keybindTargetAddress + 1) = relativeAddress;

		VirtualProtect(keybindTargetAddress, 5, oldProtect, &oldProtect);
	}
}