#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "Common/Common.hpp"
#include "Common/Semver.hpp"
#include "Enums/DynamicLinkResult.hpp"
#include "Models/DynamicLinkModuleModel.hpp"
#include "Descriptors/DynamicLinkModuleDescriptor.hpp"
#include <unordered_map>
#include <filesystem>
#include <memory>
namespace fs = std::filesystem;

extern "C" LOADER_API HMODULE WINAPI LoadDynamicLinkLibrary(const std::string& moduleFile, const std::vector<std::string>& dynamicLinkFiles);
namespace DynaLink {
	class DynamicHandle;
	class LOADER_API Loader {
		static std::unordered_map<HMODULE, std::shared_ptr<DynamicHandle>> loadedDynamicModules;
	public:
		static std::weak_ptr<DynamicHandle> LoadDynamicLinkLibrary(const std::string& moduleFile, const std::vector<std::string>& dynamicLinkFiles);
		static Semver GetModuleVersion(HMODULE moduleHandle);
		static void DynamicallyLinkModule(DynamicHandle& dynamicModule);
		static void GetDynamicLinkModuleDescriptors(DynamicHandle& dynamicModule);
		static std::vector<std::string> GetDllSearchPaths();
		static std::string GetCurrentArchitecture();
		static std::string ParseArchitecture(const std::string& architecture);
		static void WINAPI OnLoadLibraryA(const char* name);
		static void WINAPI OnFreeLibrary(HMODULE library);
		static bool IsDynamicLinkModule(HMODULE moduleHandle);
		static DynamicLinkResult ValidateDynamicLinking(DynamicHandle& dynamicModule);
		static bool ParseDynamicLinkFiles(DynamicHandle& module, const std::vector<std::string>& paths);
		static bool TryToFindDllOnStandardPaths(const std::string& moduleFile, std::string* fixedModuleFilepath);
	};
}