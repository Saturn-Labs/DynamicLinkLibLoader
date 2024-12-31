#pragma once


#include <windows.h>
#include <string>
#include <vector>
#include "Common/Common.hpp"
#include "Common/Semver.hpp"
#include "Models/DynamicLinkModuleModel.hpp"
#include "Descriptors/DynamicLinkModuleDescriptor.hpp"
#include <filesystem>
namespace fs = std::filesystem;

namespace DynaLink {
	class DynamicModule;
	class LOADER_API Loader {
	public:
		static DynamicModule LoadDynamicLinkLibrary(const std::string& moduleFile, const std::vector<std::string>& dynamicLinkFiles);
		static Semver GetModuleVersion(HMODULE moduleHandle);
		static bool DynamicallyLinkModule(DynamicModule& dynamicModule);
		static std::vector<DynamicLinkModuleDescriptor> GetDynamicLinkModuleDescriptors(DynamicModule& dynamicModule);
		static std::vector<std::string> GetDllSearchPaths();
	};
}