#include "Patches.h"
#include <HookCrashers.h>
#include "../Core/AddonCharacterConfig.h"

namespace CustomSave {
	void ApplyPatches() {
		int N = AddonCharacterConfig::getInstance().getAddonCount();
		int total = 73 + N;           // slot table size
		int split = 33 + N;          // base/workshop split (0x21 + N)
		int tableBytes = total * 4;  // memset size
		int keybindsOffset = 64 + ((TOTAL_ORIGINAL_CHARS + N) * 48);
		int newSize = 0x850 + 48 * N; // 2128 + 48*N
		int newCapacity = 0x84C + 48 * N; // 2124 + 48*N
		int addonBytes = 48 * N;
		int totalBase = 32 + N;
		int newKeybindsLimit = 0x820 + addonBytes;
		int newCharacterTableBufferSize = 14728 + (N * 32);


		HookCrashers::LogInfo("[CustomSave] Applying patches for " + std::to_string(N) + " addon characters...");

		// Create Default Characters - unlock count
		HookCrashers::PatchBytes(0x12D05B + 3, { (uint8_t)(TOTAL_ORIGINAL_CHARS + N) });

		// Character Table Allocation (LobbyManagerConstructor)
		HookCrashers::PatchBytes(0x830FE + 2, { (uint8_t)total });
		HookCrashers::PatchBytes(0x83103 + 1, { (uint8_t)total });
		HookCrashers::PatchBytes(0x8313F + 1, { (uint8_t)(tableBytes & 0xFF), (uint8_t)(tableBytes >> 8) });

		// Rebuild Character Slot Table
		HookCrashers::PatchBytes(0x84760 + 1, { (uint8_t)(tableBytes & 0xFF), (uint8_t)(tableBytes >> 8) });
		HookCrashers::PatchBytes(0x8481B + 2, { (uint8_t)split });
		HookCrashers::PatchBytes(0x84A8A + 2, { (uint8_t)total });
		HookCrashers::PatchBytes(0x84830 + 2, { (uint8_t)(-totalBase) });

		// Validate and Revoke DLC
		HookCrashers::PatchBytes(0x8744F + 2, { (uint8_t)total });
		HookCrashers::PatchBytes(0x8732B, { 0x90, 0x90, 0x90, 0x90, 0x90 });


		// 1. Patch del CMP (Check capacità buffer)
		HookCrashers::PatchBytes(0x12D344 + 3, {
			(uint8_t)(newKeybindsLimit),
			(uint8_t)(newKeybindsLimit >> 8),
			(uint8_t)(newKeybindsLimit >> 16),
			(uint8_t)(newKeybindsLimit >> 24)
			});

		// 2. Patch del MOV (Reset del cursore)
		HookCrashers::PatchBytes(0x12D34D + 3, {
			(uint8_t)(newKeybindsLimit),
			(uint8_t)(newKeybindsLimit >> 8),
			(uint8_t)(newKeybindsLimit >> 16),
			(uint8_t)(newKeybindsLimit >> 24)
			});

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

		// IsSaveBufferUninitialized
		HookCrashers::PatchBytes(0xFB2C5 + 1, {
			(uint8_t)(newSize),
			(uint8_t)(newSize >> 8),
			(uint8_t)(newSize >> 16),
			(uint8_t)(newSize >> 24)
			});

		HookCrashers::PatchBytes(0xFB2F7 + 3, {
			(uint8_t)(newCapacity),
			(uint8_t)(newCapacity >> 8),
			(uint8_t)(newCapacity >> 16),
			(uint8_t)(newCapacity >> 24)
			});

		HookCrashers::PatchBytes(0xFB2EF + 1, {
			(uint8_t)(newSize),
			(uint8_t)(newSize >> 8),
			(uint8_t)(newSize >> 16),
			(uint8_t)(newSize >> 24)
			});

		int newKeybinds = 0x820 + addonBytes;
		HookCrashers::PatchBytes(0x12EE85 + 2, {
			(uint8_t)(newKeybinds), (uint8_t)(newKeybinds >> 8),
			(uint8_t)(newKeybinds >> 16), (uint8_t)(newKeybinds >> 24)
			});

		// GenerateDefaultKeybinds (0x820)
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


		// Validation size check (lol)
		HookCrashers::PatchBytes(0x12EDB0, { 0xB0, 0x01, 0xC3, 0x90, 0x90, 0x90 });

		// Back size check save
		HookCrashers::PatchBytes(0xE70B3, { 0xEB, 0x2B, 0x90, 0x90, 0x90, 0x90 });
		HookCrashers::PatchBytes(0x12EE06 + 2, {
			(uint8_t)(newSize),
			(uint8_t)(newSize >> 8),
			(uint8_t)(newSize >> 16),
			(uint8_t)(newSize >> 24)
			});

		// IsCharacterUnlockedForPlayer
		HookCrashers::PatchBytes(0x10354A + 2, { (uint8_t)(totalBase - 1) }); // Default 31 (0x1F)
		HookCrashers::PatchBytes(0x103573 + 2, { (uint8_t)(-totalBase) }); // Default - 32 (0xE0)
		HookCrashers::PatchBytes(0x1035A9 + 2, { (uint8_t)(totalBase + 1) }); // Default 33 (0x21)
		HookCrashers::PatchBytes(0x102CD7 + 0x8E0 + 2, { (uint8_t)(totalBase + 10 + 1) }); // Default 43 (0x2B)
		HookCrashers::PatchBytes(0x102CE5 + 0x8E0 + 2, { (uint8_t)(totalBase + 20 + 1) }); // Default 53 (0x35)
		HookCrashers::PatchBytes(0x102CF3 + 0x8E0 + 2, { (uint8_t)(totalBase + 30 + 1) }); // Default 63 (0x3F)
		HookCrashers::PatchBytes(0x102CDC + 0x8E0 + 2, { (uint8_t)(totalBase + 20 + 1) }); // Default 53 (0x35)
		HookCrashers::PatchBytes(0x102CEA + 0x8E0 + 2, { (uint8_t)(totalBase + 30 + 1) }); // Default 63 (0x3F)
		HookCrashers::PatchBytes(0x102CFC + 0x8E0 + 2, { (uint8_t)(total) }); // Default 73 (0x49)
		HookCrashers::PatchBytes(0x102CCE + 0x8E0 + 2, { (uint8_t)(totalBase + 10 + 1) }); // Default 43 (0x2B)

		// AttachSkinGraphic
		HookCrashers::PatchBytes(0x8D683 + 2, { (uint8_t)(totalBase + 1) });
		HookCrashers::PatchBytes(0x8D652 + 2, { (uint8_t)(total) });
		HookCrashers::PatchBytes(0x8D659 + 2, { (uint8_t)(total) });

		// AttachWorkshopSkin
		HookCrashers::PatchBytes(0x8D29A + 2, { (uint8_t)(totalBase + 1) });

		HookCrashers::PatchBytes(0x10A684 + 1, {
			(uint8_t)(newCharacterTableBufferSize),
			(uint8_t)(newCharacterTableBufferSize >> 8),
			(uint8_t)(newCharacterTableBufferSize >> 16),
			(uint8_t)(newCharacterTableBufferSize >> 24)
			});

	};
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