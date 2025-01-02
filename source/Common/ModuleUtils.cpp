#include "Common/ModuleUtils.hpp"
#include "Common/Log.hpp"
#include "Helpers/Helpers.hpp"
#include <psapi.h>
#include <winternl.h>
#include <ntstatus.h>

namespace DynaLink {
	bool ModuleUtils::IsModuleValid(HMODULE moduleHandle) {
		char moduleFileName[MAX_PATH];
		if (!moduleHandle || (GetModuleFileNameA(moduleHandle, moduleFileName, MAX_PATH) == 0 && (GetLastError() == ERROR_MOD_NOT_FOUND || GetLastError() == ERROR_INVALID_PARAMETER))) {
			return false;
		}
		return true;
	}

	std::vector<HMODULE> ModuleUtils::GetLoadedModules() {
		std::vector<HMODULE> loadedModules{};
		DWORD processId = GetCurrentProcessId();
		HMODULE hModules[1024];
		DWORD cbNeeded;
		if (EnumProcessModules(GetCurrentProcess(), hModules, sizeof(hModules), &cbNeeded)) {
			for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
				loadedModules.push_back(hModules[i]);
			}
		}
		return loadedModules;
	}

	IMAGE_SECTION_HEADER* ModuleUtils::GetDynamicLinkSection(HMODULE moduleHandle)
	{
		if (!IsModuleValid(moduleHandle)) {
			return nullptr;
		}

		IMAGE_NT_HEADERS* ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<uintptr_t>(moduleHandle) + reinterpret_cast<IMAGE_DOS_HEADER*>(moduleHandle)->e_lfanew);
		uint16_t sectionCount = ntHeaders->FileHeader.NumberOfSections;
		IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeaders);
		for (uint16_t i = 0; i < sectionCount; i++) {
			if (strcmp(reinterpret_cast<const char*>(section->Name), ".dlink") == 0) {
				return section;
			}
			++section;
		}
		return nullptr;
	}
}