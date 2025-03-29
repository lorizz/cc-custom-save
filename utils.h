#pragma once
#include <codecvt>
#include <iomanip>
#include <sstream>
#include "logger.h"
#include <string>
#include <algorithm>
#include <cctype>

std::string IntToHex(uintptr_t value);
bool SetMemoryProtection(void* address, SIZE_T size, DWORD newProtect, DWORD* oldProtect);
uint32_t ReadLittleEndian(uintptr_t address, Logger& l);
std::wstring ToWString(const std::string& str);
const wchar_t* ToWChar(const std::string& str);

inline std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }
    auto end = s.end();
    while (end != start && std::isspace(*(end - 1))) {
        end--;
    }
    return std::string(start, end);
}