#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include "utils.h"
#include "logger.h"

struct FieldInfo {
	std::string name;
	size_t length;
	size_t offset;
};

inline std::vector<FieldInfo> readCharacterDataStructure(const std::string& filename) {
    std::vector<FieldInfo> structure;
    Logger& l = Logger::Instance();
    l.Get()->info("Attempting to open INI file: {}", filename);
    l.Get()->flush();

    std::ifstream iniFile(filename);
    if (!iniFile.is_open()) {
        l.Get()->error("Cannot open INI file: {}", filename);
        l.Get()->flush();
        return structure;
    }

    l.Get()->info("INI file opened successfully.");
    l.Get()->flush();

    std::string line;
    bool inSection = false;
    size_t cumulativeOffset = 0;

    while (std::getline(iniFile, line)) {
        l.Get()->info("Read line: '{}'", line);
        l.Get()->flush();

        if (line.empty()) continue;
        // Skip comments
        if (line[0] == ';' || line[0] == '#') continue;

        if (!inSection) {
            if (line.find("[CharacterDataStructure]") != std::string::npos) {
                inSection = true;
                l.Get()->info("Found [CharacterDataStructure] section.");
                l.Get()->flush();
            }
            continue;
        }

        // If we reach a new section, stop processing
        if (line[0] == '[') {
            l.Get()->info("Reached new section, stopping INI parsing.");
            l.Get()->flush();
            break;
        }

        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, ',') && std::getline(iss, value)) {
            try {
                size_t len = std::stoul(value);
                FieldInfo info{ key, len, cumulativeOffset };
                structure.push_back(info);
                l.Get()->info("Parsed field: '{}' with length {} at offset {}.", key, len, cumulativeOffset);
                cumulativeOffset += len;
                l.Get()->flush();
            }
            catch (const std::exception& e) {
                l.Get()->error("Error parsing line: '{}'. Exception: {}", line, e.what());
                l.Get()->flush();
            }
        }
        else {
            l.Get()->error("Failed to parse line: '{}'", line);
            l.Get()->flush();
        }
    }

    iniFile.close();
    l.Get()->info("Finished reading INI file. Total fields: {}. Total bytes: {}.", structure.size(), cumulativeOffset);
    l.Get()->flush();

    return structure;
}

inline size_t getTotalCharacterDataBytes(const std::vector<FieldInfo>& structure) {
	if (structure.empty()) return 0;
	const FieldInfo& last = structure.back();
	return last.offset + last.length;
}

inline bool getFieldInfoFromOffset(const std::vector<FieldInfo>& structure, size_t offset, std::string& fieldName, size_t& internalIndex) {
    Logger& l = Logger::Instance();
	for (const auto& field : structure) {
        l.Get()->info("Field: {}, Length: {}, Offset: {}", field.name, field.length, field.offset);
        l.Get()->flush();
		if (offset >= field.offset && offset < field.offset + field.length) {
			fieldName = field.name;
			internalIndex = offset - field.offset;
			return true;
		}
	}
	return false;
}
