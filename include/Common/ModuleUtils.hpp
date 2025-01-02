#pragma once
#include "Common/Common.hpp"
#include <windows.h>

namespace DynaLink {
	class ModuleUtils {
	public:
		static bool IsModuleValid(HMODULE moduleHandle);
		static std::vector<HMODULE> GetLoadedModules();
		static IMAGE_SECTION_HEADER* GetDynamicLinkSection(HMODULE moduleHandle);
	};
}