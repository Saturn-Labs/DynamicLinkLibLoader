#pragma once
#include "Common/Common.hpp"
#include <windows.h>

namespace DynaLink {
	enum class DynamicLinkLoadingResult : uint32_t {
		ModuleLoaded,
		ModuleLoadedLinkPartial,
		AlreadyLoaded,
		ModuleReloaded,
		InvalidModule,
		NotDynaLinkModule,
		Unknown
	};

	class DynamicHandle;
	class LOADER_API DynamicLoader {
	public:
		static std::weak_ptr<DynamicHandle> LoadDynamicLinkLibrary(const std::string& moduleFile, const std::vector<std::string>& dynamicLinkFiles, DynamicLinkLoadingResult* result = nullptr);

	private:
		static std::unordered_map<HMODULE, std::shared_ptr<DynamicHandle>> dynamicHandles;
	};
}

extern "C" LOADER_API HMODULE WINAPI LoadDynamicLinkLibrary(const std::string& moduleFile, const std::vector<std::string>& dynamicLinkFiles, DynaLink::DynamicLinkLoadingResult* result = nullptr);