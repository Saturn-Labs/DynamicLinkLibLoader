#pragma once
#ifdef LOADER_EXPORTS
#define LOADER_API __declspec(dllexport)
#else
#define LOADER_API __declspec(dllimport)
#endif

#include <windows.h>
#include <string>
#include <vector>
#include "Common/Models/DynamicLinkModuleModel.hpp"
#include "Common/Descriptors/DynamicLinkModuleDescriptor.hpp"
#include <filesystem>
namespace fs = std::filesystem;

namespace DynaLink {
	class DynamicModule;
	class LOADER_API Loader {
	public:
		static DynamicModule LoadDynamicLinkLibrary(const std::string& moduleFile, const std::vector<std::string>& dynamicLinkFiles);
		static bool DynamicallyLinkModule(DynamicModule& dynamicModule, );
	};
}