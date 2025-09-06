#include "Patches.h"
#include <HookCrashers.h>
#include "../Core/CustomSaveManager.h"

using CustomSave::CustomSaveManager;

namespace CustomSave {
	void ApplyPatches() {
		// IsCharacterUnlockedForPlayer
		HookCrashers::PatchBytes(0x102C6A + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS - 1) }); // Default 31 (0x1F)
		HookCrashers::PatchBytes(0x102C93 + 2, { (uint8_t)(-CustomSaveManager::NUM_BASE_CHARACTERS) }); // Default - 32 (0xE0)
		HookCrashers::PatchBytes(0x102CC9 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)
		HookCrashers::PatchBytes(0x102CD7 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 10 + 1) }); // Default 43 (0x2B)
		HookCrashers::PatchBytes(0x102CE5 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 20 + 1) }); // Default 53 (0x35)
		HookCrashers::PatchBytes(0x102CF3 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 30 + 1) }); // Default 63 (0x3F)
		HookCrashers::PatchBytes(0x102CDC + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 20 + 1) }); // Default 53 (0x35)
		HookCrashers::PatchBytes(0x102CEA + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 30 + 1) }); // Default 63 (0x3F)
		HookCrashers::PatchBytes(0x102CFC + 2, { (uint8_t)(CustomSaveManager::TOTAL_STREAMED_CHARACTERS + 1) }); // Default 73 (0x49)
		HookCrashers::PatchBytes(0x102CCE + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 10 + 1) }); // Default 43 (0x2B)

		// FUN_00a9e2a0
		HookCrashers::PatchBytes(0xEE2CD + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS - 1) }); // Default 31 (0x1F)

		// IsCharacterAvailableInGameMode
		HookCrashers::PatchBytes(0x8DC06 + 2, { (uint8_t)(CustomSaveManager::TOTAL_STREAMED_CHARACTERS + 1) }); // Default 73 (0x49)

		// GetSpecialCharacterIdForTeam
		HookCrashers::PatchBytes(0x8DC5A + 3, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)
		HookCrashers::PatchBytes(0x8DC63 + 2, { (uint8_t)(CustomSaveManager::TOTAL_STREAMED_CHARACTERS + 1) }); // Default 73 (0x49)

		// InitCharDataTable
		HookCrashers::PatchBytes(0x83C40 + 1, { (uint8_t)(0x124 + (CustomSaveManager::NUM_BASE_CHARACTERS - 32) * 4), 0x01, 0x00, 0x00 }); // 0x124 + (num_addon_chars * 4)
		HookCrashers::PatchBytes(0x83CFB + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)
		HookCrashers::PatchBytes(0x83D10 + 2, { (uint8_t)(-CustomSaveManager::NUM_BASE_CHARACTERS) }); // Default - 32 (0xE0)
		HookCrashers::PatchBytes(0x83F6A + 2, { (uint8_t)(CustomSaveManager::TOTAL_STREAMED_CHARACTERS + 1) }); // Default 73 (0x49)

		// SetFlagCharIsWorkshop
		HookCrashers::PatchBytes(0x36720 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS) }); // Default 32 (0x20)
		HookCrashers::PatchBytes(0x36736 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)

		// AttachSkinGraphic
		HookCrashers::PatchBytes(0x8CB32 + 2, { (uint8_t)(CustomSaveManager::TOTAL_STREAMED_CHARACTERS + 1) }); // Default 73 (0x49)
		HookCrashers::PatchBytes(0x8CB39 + 2, { (uint8_t)(CustomSaveManager::TOTAL_STREAMED_CHARACTERS + 1) }); // Default 73 (0x49)
		HookCrashers::PatchBytes(0x8CB63 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)

		// FUN_00a3d3e0
		//HookCrashers::PatchBytes(0x8D862 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)

		// SetupCharacterGraphic
		/*HookCrashers::PatchBytes(0x8D0F9 + 3, {(uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 3)}); // Default 35 (0x23)
		HookCrashers::PatchBytes(0x8D104 + 6, { (uint8_t)(0xFF - CustomSaveManager::NUM_BASE_CHARACTERS) }); // Default 223 (0xDF) // TODO CHECK CORRECTLY

		// ParseWorkshopCharactersFile
		// HookCrashers::PatchBytes(0x36414 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS) }); // Default 32 (0x20)
		// HookCrashers::PatchBytes(0x3642A + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)

		// FUN_00a36bc0
		/*HookCrashers::PatchBytes(0x86CF0 + 2, {(uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1)}); // Default 33 (0x21)
		HookCrashers::PatchBytes(0x86F59 + 2, { (uint8_t)(CustomSaveManager::TOTAL_STREAMED_CHARACTERS + 1) }); // Default 73 (0x49)

		// FUN_009ec460

		// FUN_00a0fcf0
		HookCrashers::PatchBytes(0x5FD3A + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)

		// FUN_00a33040
		HookCrashers::PatchBytes(0x8353B + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)

		// FUN_00a37010
		HookCrashers::PatchBytes(0x8712A + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)

		// FUN_00a37340
		HookCrashers::PatchBytes(0x8736A + 1, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)
		HookCrashers::PatchBytes(0x87515 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)
		HookCrashers::PatchBytes(0x87496 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)
		HookCrashers::PatchBytes(0x8755E + 2, { (uint8_t)(CustomSaveManager::TOTAL_STREAMED_CHARACTERS + 1) }); // Default 73 (0x49)

		// FUN_00a37c10
		HookCrashers::PatchBytes(0x87FC3 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)

		// FUN_00a3c730
		HookCrashers::PatchBytes(0x8C77A + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)
		HookCrashers::PatchBytes(0x8C7A3 + 2, { (uint8_t)(-CustomSaveManager::NUM_BASE_CHARACTERS) }); // Default - 32 (0xE0)
		HookCrashers::PatchBytes(0x8C7AA + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)

		// FUN_00a3de00
		HookCrashers::PatchBytes(0x8DF02 + 2, { (uint8_t)(CustomSaveManager::NUM_BASE_CHARACTERS + 1) }); // Default 33 (0x21)
		HookCrashers::PatchBytes(0x8DEE0 + 2, { (uint8_t)(CustomSaveManager::TOTAL_STREAMED_CHARACTERS + 1) }); // Default 73 (0x49)*/
	}
}