#pragma once
#include "Common/Common.hpp"

namespace DynaLink {
	struct DynamicModule {
		void* moduleHandle;
		std::string moduleFile;
		std::vector<DynamicLinkModuleModel> dynamicLinkModules;
		std::vector<DynamicLinkModuleModel> loadedDynamicLinkModules;
		std::vector<DynamicLinkModuleModel> unloadedDynamicLinkModules;

		bool IsValid() const {
			return moduleHandle != nullptr && moduleFile != "" && GetModuleHandle(fs::path(moduleFile).filename().string().c_str());
		}

		void Free() {
			if (IsValid()) {
				FreeLibrary(reinterpret_cast<HMODULE>(moduleHandle));
				moduleHandle = nullptr;
			}
		}
	};
}