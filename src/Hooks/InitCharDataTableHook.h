#pragma once

#include "../stdafx.h"
#include "AttachSkinGraphicHook.h"

namespace CustomSave {
	void ResetInitCharDataTableState();
	bool SetupInitCharDataTableHook(uintptr_t moduleBase);
}