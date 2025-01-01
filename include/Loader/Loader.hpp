#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "Common/Common.hpp"
#include "Common/Semver.hpp"
#include "Models/DynamicLinkModuleModel.hpp"
#include "Descriptors/DynamicLinkModuleDescriptor.hpp"
#include <filesystem>
#include <memory>
namespace fs = std::filesystem;

namespace DynaLink {
	class DynamicModule;
	class LOADER_API Loader {
		friend struct DynamicModule;
		static std::vector<std::shared_ptr<DynamicModule>> loadedDynamicModules;
	public:
		static std::weak_ptr<DynamicModule> LoadDynamicLinkLibrary(const std::string& moduleFile, const std::vector<std::string>& dynamicLinkFiles);
		static Semver GetModuleVersion(HMODULE moduleHandle);
		static bool DynamicallyLinkModule(DynamicModule& dynamicModule);
		static std::vector<DynamicLinkModuleDescriptor> GetDynamicLinkModuleDescriptors(DynamicModule& dynamicModule);
		static std::vector<std::string> GetDllSearchPaths();
		static std::string GetCurrentArchitecture();
		static std::string ParseArchitecture(const std::string& architecture);
		static void WINAPI OnLoadLibraryA(const char* name);
		static void WINAPI OnFreeLibrary(HMODULE library);
		static bool IsModuleValid(HMODULE moduleHandle);
	};
}