#pragma once
#include <codecvt>
#include <iomanip>
#include <sstream>
#include "logger.h"

std::string IntToHex(uintptr_t value);
bool SetMemoryProtection(void* address, SIZE_T size, DWORD newProtect, DWORD* oldProtect);
uint32_t ReadLittleEndian(uintptr_t address, Logger& l);
std::wstring ToWString(const std::string& str);
const wchar_t* ToWChar(const std::string& str);