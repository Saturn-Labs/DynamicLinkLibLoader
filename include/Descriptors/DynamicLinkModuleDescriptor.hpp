#pragma once
#include "Descriptors/DynamicLinkImportDescriptor.hpp"
#include "Loader/DynamicModule.hpp"
#include "Common/Common.hpp"
#include <Windows.h>

namespace DynaLink {
	struct LOADER_API DynamicLinkModuleDescriptor {
		std::string moduleName;
		void* moduleBase = nullptr;
		IMAGE_IMPORT_DESCRIPTOR moduleImportDescriptor = { 0 };
		std::vector<DynamicLinkImportDescriptor> importDescriptors = {};

		bool IsValid() const;
		bool HasDynamicImports() const;
		std::string GetImportModuleName() const;
		static std::optional<DynamicLinkModuleDescriptor> Create(const std::string& moduleName, void* moduleBase, const IMAGE_IMPORT_DESCRIPTOR& dynamicImportDescriptor);
		bool Equals(const DynamicLinkModuleDescriptor& other) const;
		bool operator==(const DynamicLinkModuleDescriptor& other) const;
		bool operator!=(const DynamicLinkModuleDescriptor& other) const;
	};
}