#pragma once
#include "Common/Descriptors/DynamicLinkImportDescriptor.hpp"
#include "Common/Loader/DynamicModule.hpp"
#include "Common/Common.hpp"
#include <Windows.h>

namespace DynaLink {
	struct LOADER_API DynamicLinkModuleDescriptor {
		DynamicModule& module;
		IMAGE_IMPORT_DESCRIPTOR moduleImportDescriptor = { 0 };
		std::vector<DynamicLinkImportDescriptor> importDescriptors = {};

		bool IsValid() const;
		bool HasDynamicImports() const;
		const char* GetImportModuleName() const;
		static std::optional<DynamicLinkModuleDescriptor> Create(DynamicModule& module, const IMAGE_IMPORT_DESCRIPTOR& dynamicImportDescriptor);
		bool Equals(const DynamicLinkModuleDescriptor& other) const;
		bool operator==(const DynamicLinkModuleDescriptor& other) const;
		bool operator!=(const DynamicLinkModuleDescriptor& other) const;
	};
}