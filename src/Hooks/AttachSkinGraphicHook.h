#pragma once

#include "../stdafx.h"

namespace CustomSave {
	bool SetupAttachSkinGraphicHook(uintptr_t moduleBase);
	void ResetGraphicShiftState();
}