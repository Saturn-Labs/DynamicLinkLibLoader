#pragma once
#include "Common/Common.hpp"
#include "Common/Semver.hpp"
#include <windows.h>

namespace DynaLink {
	class ModuleUtils {
	public:
		static bool IsModuleValid(HMODULE moduleHandle);
		static std::vector<HMODULE> GetLoadedModules();
		static IMAGE_SECTION_HEADER* GetDynamicLinkSection(HMODULE moduleHandle);
		static bool IsDynaLinkModuleFile(std::ifstream& stream);
		static bool IsDynaLinkModuleFile(const std::string& moduleFile);
		static std::vector<std::string> GetModuleStandardSearchPaths();
		static bool FindModuleOnStandardPaths(const std::string& moduleFile, std::string* newPath = nullptr);
		static Semver GetModuleVersion(HMODULE moduleHandle);
	};
}