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
const std::string KEYBINDS_FILENAME = "scripts/saves/keybinds.json";

static int keybindWriteCurrentIndex = 0;

static std::vector<int> keybindValues(18, 0);

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

uint8_t ReadByteFromJson(uint32_t offset) {
	Logger& l = Logger::Instance();
	l.Get()->info("Reading byte {}: ", offset);
	l.Get()->flush();

	uint8_t byteValue = 0;
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
			l.Get()->info("Reading addo character {}, {}", characterIndex, characterId);
			l.Get()->flush();

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

void WriteKeybindToJson(uintptr_t offset, uintptr_t byteValue) {
	Logger& l = Logger::Instance();
	l.Get()->info("WriteKeybindToJson called with offset {} byteValue {}", offset, byteValue);

	// Salva il valore nell'array
	keybindValues[keybindWriteCurrentIndex] = (int)byteValue;
	keybindWriteCurrentIndex++;

	// Quando raggiungiamo l'ultimo indice, scriviamo su file JSON
	if (keybindWriteCurrentIndex >= 18) {
		try {
			// Creare l'oggetto JSON
			json keybindsJson;

			// Mappa dei nomi dei tasti in base all'indice
			const std::vector<std::string> keyNames = {
				"MOVE_LEFT", "MOVE_RIGHT", "MOVE_UP", "MOVE_DOWN", "LIGHT_ATTACK", "HEAVY_ATTACK", "USE_ITEM", "JUMP", "MAGIC", "ITEM_SELECT_BACKWARD",
				"ITEM_SELECT_FORWARD", "PAUSE", "BLOCK", "UNK_2", "RETURN_HOME", "UNK_1", "STATS", "UNK_3"
			};

			// Aggiungi tutti i valori al JSON
			for (int i = 0; i < 18; i++) {
				l.Get()->info("Writing to file {} with {}", keyNames[i], keybindValues[i]);
				keybindsJson[keyNames[i]] = keybindValues[i];
			}

			// Scrivi il JSON su file
			std::ofstream outFile(KEYBINDS_FILENAME);
			if (outFile.is_open()) {
				outFile << keybindsJson.dump(4); // 4 spazi di indentazione per leggibilità
				outFile.close();
				l.Get()->info("Keybinds saved to {}", KEYBINDS_FILENAME);
			}
			else {
				l.Get()->error("Failed to open file for writing: {}", KEYBINDS_FILENAME);
			}
		}
		catch (const std::exception& e) {
			l.Get()->error("Error writing keybinds to JSON: {}", e.what());
		}

		// Reset dell'indice
		keybindWriteCurrentIndex = 0;
	}

	l.Get()->flush();
}

// Restituisce il valore keybind dal file JSON, o un valore predefinito se non trovato
int GetKeybindValue(const std::string& keyName, int defaultValue = 0) {
	const std::string KEYBINDS_FILENAME = "scripts/saves/keybinds.json";
	Logger& l = Logger::Instance();

	try {
		// Prova ad aprire il file JSON
		std::ifstream file(KEYBINDS_FILENAME);
		if (!file.is_open()) {
			l.Get()->warn("Keybinds file not found: {}", KEYBINDS_FILENAME);
			return defaultValue;
		}

		// Carica il contenuto JSON
		nlohmann::json keybindsJson;
		file >> keybindsJson;

		// Controlla se la chiave esiste
		if (keybindsJson.contains(keyName)) {
			return keybindsJson[keyName].get<int>();
		}
		else {
			l.Get()->warn("Key '{}' not found in keybinds file", keyName);
			return defaultValue;
		}
	}
	catch (const std::exception& e) {
		l.Get()->error("Error reading keybind '{}': {}", keyName, e.what());
		return defaultValue;
	}
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

	// Copia l'opcode "mov eax, [memoria]" che è A1
	*(BYTE*)(codeCave + offset) = 0xA1;
	offset += 1;

	// Copia l'indirizzo a cui punta l'istruzione
	DWORD addressValue = *(DWORD*)(base + 0x98CC8 + 1); // +1 per saltare l'opcode
	*(DWORD*)(codeCave + offset) = addressValue;
	offset += 4;

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

	// mov esi, dword ptr ds:[edx + 0xC] -> @param offset
	codeCave[offset++] = 0x8B;
	codeCave[offset++] = 0x72;
	codeCave[offset++] = 0x0C;

	// cmp esi, dword ptr ds:[edx + 8]
	codeCave[offset++] = 0x3B;
	codeCave[offset++] = 0x72;
	codeCave[offset++] = 0x08;

	// jb label_87AEE6
	codeCave[offset++] = 0x73;
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

	// mov al, byte ptr ss:[ebp + 0x10] -> @param byteValue
	codeCave[offset++] = 0x8A;
	codeCave[offset++] = 0x45;
	codeCave[offset++] = 0x10;

	// Salva i registri che utilizzerai dopo
	codeCave[offset++] = 0x51;  // push ecx
	codeCave[offset++] = 0x56;  // push esi

	// Preparazione parametri per WriteKeybindToJson
	codeCave[offset++] = 0x0F;  // movzx eax, al
	codeCave[offset++] = 0xB6;
	codeCave[offset++] = 0xC0;
	codeCave[offset++] = 0x50;  // push eax (byteValue)

	// push offset (che è in esi)
	codeCave[offset++] = 0x56;  // push esi

	// Chiamata alla funzione
	codeCave[offset++] = 0xE8;
	// Qui devi calcolare l'offset corretto alla funzione
	DWORD callOffset = (DWORD)((uintptr_t)WriteKeybindToJson - ((uintptr_t)(codeCave + offset) + 4));
	*(DWORD*)(codeCave + offset) = callOffset;
	offset += 4;

	// Pulizia dello stack dopo la chiamata (2 parametri = 8 bytes)
	codeCave[offset++] = 0x83;  // add esp, 8
	codeCave[offset++] = 0xC4;
	codeCave[offset++] = 0x08;

	// Ripristina i registri nell'ordine inverso
	codeCave[offset++] = 0x5E;  // pop esi
	codeCave[offset++] = 0x59;  // pop ecx

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
	
	std::vector<uintptr_t> keybindOffsets = {
		0x8D33B
	};

	for (uintptr_t off : keybindOffsets) {
		unsigned char* readKeybindAddress = reinterpret_cast<unsigned char*>(base + off);
		unsigned char readKeybindBytes[7] = { 0xC7, 0x41, 0x0C, 0x99, 0x99, 0x09, 0x00 };
		VirtualProtect(readKeybindAddress, sizeof(readKeybindBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(reinterpret_cast<void*>(readKeybindAddress), readKeybindBytes, sizeof(readKeybindBytes));
		VirtualProtect(readKeybindAddress, sizeof(readKeybindBytes), oldProtect, &oldProtect);
	}

	std::vector<uintptr_t> keybindOffsets2 = {
		0x8D7AD
	};

	for (uintptr_t off : keybindOffsets2) {
		unsigned char* readKeybindAddress = reinterpret_cast<unsigned char*>(base + off);
		unsigned char readKeybindBytes[7] = { 0xC7, 0x47, 0x0C, 0x99, 0x99, 0x09, 0x00 };
		VirtualProtect(readKeybindAddress, sizeof(readKeybindBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(reinterpret_cast<void*>(readKeybindAddress), readKeybindBytes, sizeof(readKeybindBytes));
		VirtualProtect(readKeybindAddress, sizeof(readKeybindBytes), oldProtect, &oldProtect);
	}

	std::vector<uintptr_t> keybindOffsets3 = {
		0x98CDF,
		0x99052
	};

	for (uintptr_t off : keybindOffsets3) {
		unsigned char* readKeybindAddress = reinterpret_cast<unsigned char*>(base + off);
		unsigned char readKeybindBytes[7] = { 0xC7, 0x40, 0x0C, 0x99, 0x99, 0x09, 0x00 };
		VirtualProtect(readKeybindAddress, sizeof(readKeybindBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(reinterpret_cast<void*>(readKeybindAddress), readKeybindBytes, sizeof(readKeybindBytes));
		VirtualProtect(readKeybindAddress, sizeof(readKeybindBytes), oldProtect, &oldProtect);
	}

	std::vector<uintptr_t> keybindResetOffsets = {
		0x991D8,
		0x9924A,
		0x992BC,
		0x9932E,
		0x993A0,
		0x99412,
		0x99484,
		0x994F6,
		0x99568,
		0x995DA,
		0x9964C,
		0x996BE,
		0x99730,
		0x997A8,
		0x9981A,
		0x9988C,
		0x99904,
		0x99976
	};

	for (uintptr_t off : keybindResetOffsets) {
		unsigned char* resetAddress = reinterpret_cast<unsigned char*>(base + off);
		unsigned char resetBytes[2] = { 0x75, 0x19 };
		VirtualProtect(resetAddress, sizeof(resetBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(reinterpret_cast<void*>(resetAddress), resetBytes, sizeof(resetBytes));
		VirtualProtect(resetAddress, sizeof(resetBytes), oldProtect, &oldProtect);
	}

	// JB Patch here
	std::vector<uintptr_t> keybindStartupOffsets = {
		0x8D7C8,
		0x8D84E,
		0x8D8CC,
		0x8D94A,
		0x8D9C8,
		0x8DA18,
		0x8DA68,
		0x8DAB8,
		0x8DB08,
		0x8DB58,
		0x8DBA8,
		0x8DBF8,
		0x8DCBB,
		0x8DD0B,
		0x8DD61,
		0x8DDB1,
		0x8DE01,
		0x8DE57
	};

	for (uintptr_t off : keybindStartupOffsets) {
		unsigned char* startupAddress = reinterpret_cast<unsigned char*>(base + off);
		unsigned char startupBytes[2] = { 0x73, 0x04 };
		VirtualProtect(startupAddress, sizeof(startupBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(reinterpret_cast<void*>(startupAddress), startupBytes, sizeof(startupBytes));
		VirtualProtect(startupAddress, sizeof(startupBytes), oldProtect, &oldProtect);
	}

	auto storageCodeOffset = InjectStorageCode(base);

	std::vector<uintptr_t> keybindWriteOffsets = {
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

	for (uintptr_t off : keybindWriteOffsets) {
		unsigned char* keybindTargetAddress = reinterpret_cast<unsigned char*>(base + off);
		DWORD oldProtect;
		VirtualProtect(keybindTargetAddress, 5, PAGE_EXECUTE_READWRITE, &oldProtect);

		*keybindTargetAddress = 0xE8; // Opcode per CALL
		uintptr_t relativeAddress = reinterpret_cast<uintptr_t>(storageCodeOffset) - reinterpret_cast<uintptr_t>(keybindTargetAddress) - 5;
		*reinterpret_cast<uintptr_t*>(keybindTargetAddress + 1) = relativeAddress;

		VirtualProtect(keybindTargetAddress, 5, oldProtect, &oldProtect);
	}

	// Patching on startup

	int moveLeftValue = GetKeybindValue("MOVE_LEFT", 0x25);
	unsigned char* moveLeftAddress = reinterpret_cast<unsigned char*>(base + 0x8D7D1);
	unsigned char moveLeftBytes[3] = { 0xB3, static_cast<unsigned char>(moveLeftValue), 0x90};
	VirtualProtect(moveLeftAddress, sizeof(moveLeftBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(moveLeftAddress), moveLeftBytes, sizeof(moveLeftBytes));
	VirtualProtect(moveLeftAddress, sizeof(moveLeftBytes), oldProtect, &oldProtect);

	int moveRightValue = GetKeybindValue("MOVE_RIGHT", 0x27);
	unsigned char* moveRightAddress = reinterpret_cast<unsigned char*>(base + 0x8D857);
	unsigned char moveRightBytes[3] = { 0xB3, static_cast<unsigned char>(moveRightValue), 0x90 };
	VirtualProtect(moveRightAddress, sizeof(moveRightBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(moveRightAddress), moveRightBytes, sizeof(moveRightBytes));
	VirtualProtect(moveRightAddress, sizeof(moveRightBytes), oldProtect, &oldProtect);

	int moveUpValue = GetKeybindValue("MOVE_UP", 0x26);
	unsigned char* moveUpAddress = reinterpret_cast<unsigned char*>(base + 0x8D8D5);
	unsigned char moveUpBytes[3] = { 0xB3, static_cast<unsigned char>(moveUpValue), 0x90 };
	VirtualProtect(moveUpAddress, sizeof(moveUpBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(moveUpAddress), moveUpBytes, sizeof(moveUpBytes));
	VirtualProtect(moveUpAddress, sizeof(moveUpBytes), oldProtect, &oldProtect);

	int moveDownValue = GetKeybindValue("MOVE_DOWN", 0x28);
	unsigned char* moveDownAddress = reinterpret_cast<unsigned char*>(base + 0x8D953);
	unsigned char moveDownBytes[3] = { 0xB3, static_cast<unsigned char>(moveDownValue), 0x90 };
	VirtualProtect(moveDownAddress, sizeof(moveDownBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(moveDownAddress), moveDownBytes, sizeof(moveDownBytes));
	VirtualProtect(moveDownAddress, sizeof(moveDownBytes), oldProtect, &oldProtect);

	int lightAttackValue = GetKeybindValue("LIGHT_ATTACK", 0x41);
	unsigned char* lightAttackAddress = reinterpret_cast<unsigned char*>(base + 0x8D9D1);
	unsigned char lightAttackBytes[3] = { 0xB3, static_cast<unsigned char>(lightAttackValue), 0x90 };
	VirtualProtect(lightAttackAddress, sizeof(lightAttackBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(lightAttackAddress), lightAttackBytes, sizeof(lightAttackBytes));
	VirtualProtect(lightAttackAddress, sizeof(lightAttackBytes), oldProtect, &oldProtect);

	int heavyAttackValue = GetKeybindValue("HEAVY_ATTACK", 0x57);
	unsigned char* heavyAttackAddress = reinterpret_cast<unsigned char*>(base + 0x8DA21);
	unsigned char heavyAttackBytes[3] = { 0xB3, static_cast<unsigned char>(heavyAttackValue), 0x90 };
	VirtualProtect(heavyAttackAddress, sizeof(heavyAttackBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(heavyAttackAddress), heavyAttackBytes, sizeof(heavyAttackBytes));
	VirtualProtect(heavyAttackAddress, sizeof(heavyAttackBytes), oldProtect, &oldProtect);

	int useItemValue = GetKeybindValue("USE_ITEM", 0x44);
	unsigned char* useItemAddress = reinterpret_cast<unsigned char*>(base + 0x8DA71);
	unsigned char useItemBytes[3] = { 0xB3, static_cast<unsigned char>(useItemValue), 0x90 };
	VirtualProtect(useItemAddress, sizeof(useItemBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(useItemAddress), useItemBytes, sizeof(useItemBytes));
	VirtualProtect(useItemAddress, sizeof(useItemBytes), oldProtect, &oldProtect);

	int jumpValue = GetKeybindValue("JUMP", 0x53);
	unsigned char* jumpAddress = reinterpret_cast<unsigned char*>(base + 0x8DAC1);
	unsigned char jumpBytes[3] = { 0xB3, static_cast<unsigned char>(jumpValue), 0x90 };
	VirtualProtect(jumpAddress, sizeof(jumpBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(jumpAddress), jumpBytes, sizeof(jumpBytes));
	VirtualProtect(jumpAddress, sizeof(jumpBytes), oldProtect, &oldProtect);

	int magicValue = GetKeybindValue("MAGIC", 0x45);
	unsigned char* magicAddress = reinterpret_cast<unsigned char*>(base + 0x8DB11);
	unsigned char magicBytes[3] = { 0xB3, static_cast<unsigned char>(magicValue), 0x90 };
	VirtualProtect(magicAddress, sizeof(magicBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(magicAddress), magicBytes, sizeof(magicBytes));
	VirtualProtect(magicAddress, sizeof(magicBytes), oldProtect, &oldProtect);

	int itemSelectBackwardValue = GetKeybindValue("ITEM_SELECT_BACKWARD", 0x5A);
	unsigned char* itemSelectBackwardAddress = reinterpret_cast<unsigned char*>(base + 0x8DB61);
	unsigned char itemSelectBackwardBytes[3] = { 0xB3, static_cast<unsigned char>(itemSelectBackwardValue), 0x90 };
	VirtualProtect(itemSelectBackwardAddress, sizeof(itemSelectBackwardBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(itemSelectBackwardAddress), itemSelectBackwardBytes, sizeof(itemSelectBackwardBytes));
	VirtualProtect(itemSelectBackwardAddress, sizeof(itemSelectBackwardBytes), oldProtect, &oldProtect);

	int itemSelectForwardValue = GetKeybindValue("ITEM_SELECT_FORWARD", 0x43);
	unsigned char* itemSelectForwardAddress = reinterpret_cast<unsigned char*>(base + 0x8DBB1);
	unsigned char itemSelectForwardBytes[3] = { 0xB3, static_cast<unsigned char>(itemSelectForwardValue), 0x90 };
	VirtualProtect(itemSelectForwardAddress, sizeof(itemSelectForwardBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(itemSelectForwardAddress), itemSelectForwardBytes, sizeof(itemSelectForwardBytes));
	VirtualProtect(itemSelectForwardAddress, sizeof(itemSelectForwardBytes), oldProtect, &oldProtect);

	int pauseValue = GetKeybindValue("PAUSE", 0x1B);
	unsigned char* pauseAddress = reinterpret_cast<unsigned char*>(base + 0x8DC01);
	unsigned char pauseBytes[3] = { 0xB3, static_cast<unsigned char>(pauseValue), 0x90 };
	VirtualProtect(pauseAddress, sizeof(pauseBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(pauseAddress), pauseBytes, sizeof(pauseBytes));
	VirtualProtect(pauseAddress, sizeof(pauseBytes), oldProtect, &oldProtect);

	int blockValue = GetKeybindValue("BLOCK", 0x51);
	unsigned char* blockAddress = reinterpret_cast<unsigned char*>(base + 0x8DCC4);
	unsigned char blockBytes[3] = { 0xB3, static_cast<unsigned char>(blockValue), 0x90 };
	VirtualProtect(blockAddress, sizeof(blockBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(blockAddress), blockBytes, sizeof(blockBytes));
	VirtualProtect(blockAddress, sizeof(blockBytes), oldProtect, &oldProtect);

	int unk2Value = GetKeybindValue("UNK_2", 0x41);
	unsigned char* unk2Address = reinterpret_cast<unsigned char*>(base + 0x8DD14);
	unsigned char unk2Bytes[3] = { 0xB3, static_cast<unsigned char>(unk2Value), 0x90 };
	VirtualProtect(unk2Address, sizeof(unk2Bytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(unk2Address), unk2Bytes, sizeof(unk2Bytes));
	VirtualProtect(unk2Address, sizeof(unk2Bytes), oldProtect, &oldProtect);

	int returnHomeValue = GetKeybindValue("RETURN_HOME", 0x53);
	unsigned char* returnHomeAddress = reinterpret_cast<unsigned char*>(base + 0x8DD6A);
	unsigned char returnHomeBytes[3] = { 0xB3, static_cast<unsigned char>(returnHomeValue), 0x90 };
	VirtualProtect(returnHomeAddress, sizeof(returnHomeBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(returnHomeAddress), returnHomeBytes, sizeof(returnHomeBytes));
	VirtualProtect(returnHomeAddress, sizeof(returnHomeBytes), oldProtect, &oldProtect);

	int unk1Value = GetKeybindValue("UNK_1", 0x44);
	unsigned char* unk1Address = reinterpret_cast<unsigned char*>(base + 0x8DDBA);
	unsigned char unk1Bytes[3] = { 0xB3, static_cast<unsigned char>(unk1Value), 0x90 };
	VirtualProtect(unk1Address, sizeof(unk1Bytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(unk1Address), unk1Bytes, sizeof(unk1Bytes));
	VirtualProtect(unk1Address, sizeof(unk1Bytes), oldProtect, &oldProtect);

	int statsValue = GetKeybindValue("STATS", 0x09);
	unsigned char* statsAddress = reinterpret_cast<unsigned char*>(base + 0x8DE0A);
	unsigned char statsBytes[3] = { 0xB3, static_cast<unsigned char>(statsValue), 0x90 };
	VirtualProtect(statsAddress, sizeof(statsBytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(statsAddress), statsBytes, sizeof(statsBytes));
	VirtualProtect(statsAddress, sizeof(statsBytes), oldProtect, &oldProtect);

	int unk3Value = GetKeybindValue("UNK_3", 0x48);
	unsigned char* unk3Address = reinterpret_cast<unsigned char*>(base + 0x8DE60);
	unsigned char unk3Bytes[3] = { 0xB3, static_cast<unsigned char>(unk3Value), 0x90 };
	VirtualProtect(unk3Address, sizeof(unk3Bytes), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(unk3Address), unk3Bytes, sizeof(unk3Bytes));
	VirtualProtect(unk3Address, sizeof(unk3Bytes), oldProtect, &oldProtect);
}