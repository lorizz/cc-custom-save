#include "Patches.h"
#include <HookCrashers.h>
#include "../Core/AddonCharacterConfig.h"

namespace CustomSave {
	void ApplyPatches() {
		int N = AddonCharacterConfig::getInstance().getAddonCount();

		// ============================================================
		//  Derived sizes
		// ============================================================
		int total = TOTAL_ORIGINAL_SLOTS + N;
		int split = BASE_FRESH_COUNT + 1 + N;           // base/workshop split (0x21 + N)
		int tableBytes = total * 4;        // memset size for slot table
		int newSize = BASE_SAVE_SIZE + CHAR_DATA_SIZE * N;   // save buffer size     (2128 + 48*N)
		int newCapacity = BASE_SAVE_CAPACITY + CHAR_DATA_SIZE * N;   // save buffer capacity (2124 + 48*N)
		int addonBytes = CHAR_DATA_SIZE * N;
		int totalBase = BASE_CHAR_COUNT + N;           // base char count + addons
		int newKeybindsLimit = BASE_KEYBINDS_OFFSET + addonBytes; // keybinds offset in buffer
		int newCharacterTableBufferSize = CHAR_TABLE_BASE_SIZE + CHAR_TABLE_ENTRY_SIZE * N; // d_characterTable malloc size
		int keybindsOffset = GLOBAL_UNLOCK_SIZE + (TOTAL_ORIGINAL_CHARS + N) * CHAR_DATA_SIZE;

		int p2Start = BASE_CHAR_COUNT + WORKSHOP_CHAR_COUNT + N;
		int p1WorkshopStart = totalBase;
		int safeLimit = p2Start - 1;

		HookCrashers::LogInfo("[CustomSave] Applying patches for " + std::to_string(N) + " addon characters...");

		// ============================================================
		//  1. SAVE FILE - BUFFER ALLOCATION
		//     IsSaveBufferUninitialized
		// ============================================================

		// Malloc(v2, 2128) -> newSize
		HookCrashers::PatchBytes(0xFB2C5 + 1, {
			(uint8_t)(newSize),
			(uint8_t)(newSize >> 8),
			(uint8_t)(newSize >> 16),
			(uint8_t)(newSize >> 24)
			});
		// FastMemset(v3, 0, 2128) -> newSize
		HookCrashers::PatchBytes(0xFB2EF + 1, {
			(uint8_t)(newSize),
			(uint8_t)(newSize >> 8),
			(uint8_t)(newSize >> 16),
			(uint8_t)(newSize >> 24)
			});
		// this[2] = 2124 -> newCapacity
		HookCrashers::PatchBytes(0xFB2F7 + 3, {
			(uint8_t)(newCapacity),
			(uint8_t)(newCapacity >> 8),
			(uint8_t)(newCapacity >> 16),
			(uint8_t)(newCapacity >> 24)
			});

		// ============================================================
		//  2. SAVE FILE - VALIDATION BYPASSES
		// ============================================================

		// ValidateFileSaveSize -> always return 1
		HookCrashers::PatchBytes(0x12EDB0, { 0xB0, 0x01, 0xC3, 0x90, 0x90, 0x90 });

		// sub_8C7020 (backup gen) - skip size check 2128/1600/1552
		HookCrashers::PatchBytes(0xE70B3, { 0xEB, 0x2B, 0x90, 0x90, 0x90, 0x90 });

		// sub_90EDD0 - cmp edx, 2128
		HookCrashers::PatchBytes(0x12EE06 + 2, {
			(uint8_t)(newSize),
			(uint8_t)(newSize >> 8),
			(uint8_t)(newSize >> 16),
			(uint8_t)(newSize >> 24)
			});

		// ============================================================
		//  3. KEYBINDS - CURSOR MANAGEMENT
		//     Pattern: if (buffer.size > 0x820) buffer.cursor = 0x820
		//     Both CMP and MOV patched with newKeybindsLimit = 0x820 + 48*N
		// ============================================================

		// sub_90D0B0 - LoadKeybindsFromBuffer
		HookCrashers::PatchBytes(0x12D344 + 3, {
			(uint8_t)(newKeybindsLimit),
			(uint8_t)(newKeybindsLimit >> 8),
			(uint8_t)(newKeybindsLimit >> 16),
			(uint8_t)(newKeybindsLimit >> 24)
			});
		HookCrashers::PatchBytes(0x12D34D + 3, {
			(uint8_t)(newKeybindsLimit),
			(uint8_t)(newKeybindsLimit >> 8),
			(uint8_t)(newKeybindsLimit >> 16),
			(uint8_t)(newKeybindsLimit >> 24)
			});

		// GenerateDefaultKeybinds
		HookCrashers::PatchBytes(0x12CCDC + 1, {
			(uint8_t)(newKeybindsLimit),
			(uint8_t)(newKeybindsLimit >> 8),
			(uint8_t)(newKeybindsLimit >> 16),
			(uint8_t)(newKeybindsLimit >> 24)
			});
		HookCrashers::PatchBytes(0x12CCE3 + 3, {
			(uint8_t)(newKeybindsLimit),
			(uint8_t)(newKeybindsLimit >> 8),
			(uint8_t)(newKeybindsLimit >> 16),
			(uint8_t)(newKeybindsLimit >> 24)
			});

		// sub_91B020 - ACCEPT keybind
		HookCrashers::PatchBytes(0x13B050 + 3, {
			(uint8_t)(newKeybindsLimit),
			(uint8_t)(newKeybindsLimit >> 8),
			(uint8_t)(newKeybindsLimit >> 16),
			(uint8_t)(newKeybindsLimit >> 24)
			});
		HookCrashers::PatchBytes(0x13B059 + 3, {
			(uint8_t)(newKeybindsLimit),
			(uint8_t)(newKeybindsLimit >> 8),
			(uint8_t)(newKeybindsLimit >> 16),
			(uint8_t)(newKeybindsLimit >> 24)
			});

		// sub_91B4E0 - WRITE keybind (exit settings)
		HookCrashers::PatchBytes(0x13B53B + 3, {
			(uint8_t)(newKeybindsLimit),
			(uint8_t)(newKeybindsLimit >> 8),
			(uint8_t)(newKeybindsLimit >> 16),
			(uint8_t)(newKeybindsLimit >> 24)
			});
		HookCrashers::PatchBytes(0x13B544 + 3, {
			(uint8_t)(newKeybindsLimit),
			(uint8_t)(newKeybindsLimit >> 8),
			(uint8_t)(newKeybindsLimit >> 16),
			(uint8_t)(newKeybindsLimit >> 24)
			});

		// sub_91BFA0 - REVERT keybind / open settings display
		HookCrashers::PatchBytes(0x13BFD2 + 3, {
			(uint8_t)(newKeybindsLimit),
			(uint8_t)(newKeybindsLimit >> 8),
			(uint8_t)(newKeybindsLimit >> 16),
			(uint8_t)(newKeybindsLimit >> 24)
			});
		HookCrashers::PatchBytes(0x13BFDB + 3, {
			(uint8_t)(newKeybindsLimit),
			(uint8_t)(newKeybindsLimit >> 8),
			(uint8_t)(newKeybindsLimit >> 16),
			(uint8_t)(newKeybindsLimit >> 24)
			});

		// sub_90CC40 - CheckKeybindsZero (calls GenerateDefaultKeybinds if all zero)
		HookCrashers::PatchBytes(0x12CC55 + 1, {
			(uint8_t)(0x832 + addonBytes),
			(uint8_t)((0x832 + addonBytes) >> 8),
			(uint8_t)((0x832 + addonBytes) >> 16),
			(uint8_t)((0x832 + addonBytes) >> 24)
			});
		HookCrashers::PatchBytes(0x12CC5C + 3, {
			(uint8_t)(0x832 + addonBytes),
			(uint8_t)((0x832 + addonBytes) >> 8),
			(uint8_t)((0x832 + addonBytes) >> 16),
			(uint8_t)((0x832 + addonBytes) >> 24)
			});

		// ============================================================
		//  4. CHARACTER DATA - DEFAULT CHARACTERS
		//     CreateDefaultCharacters loop limit (0x2A = 42 chars)
		// ============================================================

		HookCrashers::PatchBytes(0x12D05B + 3, { (uint8_t)(TOTAL_ORIGINAL_CHARS + N) });

		// sub_90C9C0 - ValidateCharacterStats loop limit (i < 0x821 -> i < 0x821 + addonBytes)
		HookCrashers::PatchBytes(0x12CC22 + 2, {
			(uint8_t)(0x821 + addonBytes),
			(uint8_t)((0x821 + addonBytes) >> 8),
			(uint8_t)((0x821 + addonBytes) >> 16),
			(uint8_t)((0x821 + addonBytes) >> 24)
			});

		// ============================================================
		//  5. CHARACTER TABLE (LobbyManager)
		//     d_characterTable allocation and slot table setup
		// ============================================================

		// LobbyManagerConstructor - slot table size
		HookCrashers::PatchBytes(0x830FE + 2, { (uint8_t)total });
		HookCrashers::PatchBytes(0x83103 + 1, { (uint8_t)total });
		HookCrashers::PatchBytes(0x8313F + 1, { (uint8_t)(tableBytes & 0xFF), (uint8_t)(tableBytes >> 8) });

		// RebuildCharacterSlotTable
		HookCrashers::PatchBytes(0x84760 + 1, { (uint8_t)(tableBytes & 0xFF), (uint8_t)(tableBytes >> 8) });
		HookCrashers::PatchBytes(0x8481B + 2, { (uint8_t)split });
		HookCrashers::PatchBytes(0x84A8A + 2, { (uint8_t)total });
		HookCrashers::PatchBytes(0x84830 + 2, { (uint8_t)(-totalBase) });

		// d_characterTable malloc size
		HookCrashers::PatchBytes(0x10A684 + 1, {
			(uint8_t)(newCharacterTableBufferSize),
			(uint8_t)(newCharacterTableBufferSize >> 8),
			(uint8_t)(newCharacterTableBufferSize >> 16),
			(uint8_t)(newCharacterTableBufferSize >> 24)
			});

		// ============================================================
		//  6. CHARACTER UNLOCK / SKIN LOGIC
		// ============================================================

		// ValidateAndRevokeDLC
		HookCrashers::PatchBytes(0x8744F + 2, { (uint8_t)total });
		HookCrashers::PatchBytes(0x8732B, { 0x90, 0x90, 0x90, 0x90, 0x90 });

		// IsCharacterUnlockedForPlayer
		HookCrashers::PatchBytes(0x10354A + 2, { (uint8_t)(totalBase - 1) });               // Default 31 (0x1F)
		HookCrashers::PatchBytes(0x103573 + 2, { (uint8_t)(-totalBase) });                  // Default -32 (0xE0)
		HookCrashers::PatchBytes(0x1035A9 + 2, { (uint8_t)(totalBase + 1) });               // Default 33 (0x21)
		HookCrashers::PatchBytes(0x102CD7 + 0x8E0 + 2, { (uint8_t)(totalBase + 10 + 1) }); // Default 43 (0x2B)
		HookCrashers::PatchBytes(0x102CE5 + 0x8E0 + 2, { (uint8_t)(totalBase + 20 + 1) }); // Default 53 (0x35)
		HookCrashers::PatchBytes(0x102CF3 + 0x8E0 + 2, { (uint8_t)(totalBase + 30 + 1) }); // Default 63 (0x3F)
		HookCrashers::PatchBytes(0x102CDC + 0x8E0 + 2, { (uint8_t)(totalBase + 20 + 1) }); // Default 53 (0x35)
		HookCrashers::PatchBytes(0x102CEA + 0x8E0 + 2, { (uint8_t)(totalBase + 30 + 1) }); // Default 63 (0x3F)
		HookCrashers::PatchBytes(0x102CFC + 0x8E0 + 2, { (uint8_t)(total) });              // Default 73 (0x49)
		HookCrashers::PatchBytes(0x102CCE + 0x8E0 + 2, { (uint8_t)(totalBase + 10 + 1) }); // Default 43 (0x2B)

		// AttachSkinGraphic
		HookCrashers::PatchBytes(0x8D683 + 2, { (uint8_t)(totalBase + 1) });
		HookCrashers::PatchBytes(0x8D652 + 2, { (uint8_t)(total) });
		HookCrashers::PatchBytes(0x8D659 + 2, { (uint8_t)(total) });

		// AttachWorkshopSkin
		HookCrashers::PatchBytes(0x8D29A + 2, { (uint8_t)(totalBase + 1) });
	}
}

// IsCharacterAvailableInGameMode <- decompile
		//HookCrashers::PatchBytes(0x8DC06 + 0xB20 + 2, {(uint8_t)(total)}); // Default 73 (0x49)

		// GetSpecialCharacterIdForTeam <- decompile
		//HookCrashers::PatchBytes(0x8DC5A + 0xB20 + 3, { (uint8_t)(totalBase + 1) }); // Default 33 (0x21)
		//HookCrashers::PatchBytes(0x8DC63 + 0xB20 + 2, { (uint8_t)(total + 1) }); // Default 73 (0x49)

		// SetFlagIsCharWorkshop
		// HookCrashers::PatchBytes(0x36986 + 2, { (uint8_t)(totalBase + 1) }); // Default 33 (0x21)

		// GetCharacterGameCompletedForPlayer <- decompile
		//HookCrashers::PatchBytes(0x102B54 + 0x8E0 + 2, { (uint8_t)(-(totalBase + 10)) });
		//HookCrashers::PatchBytes(0x102B64 + 0x8E0 + 2, { (uint8_t)(totalBase) });
		//HookCrashers::PatchBytes(0x102B72 + 0x8E0 + 2, { (uint8_t)(totalBase + 10) });