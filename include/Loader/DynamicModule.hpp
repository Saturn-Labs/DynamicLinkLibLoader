#pragma once
#include "Common/Common.hpp"


namespace DynaLink {
	struct DynamicLinkModuleModel;
	struct DynamicLinkModuleDescriptor;
	struct DynamicLinkImportDescriptor;
	struct DynamicModule {
		void* moduleHandle;
		std::string moduleFile;
		std::vector<DynamicLinkModuleModel> parsedDynamicLinkModules;
		std::vector<DynamicLinkModuleDescriptor> dynamicLinkModuleDescriptors;
		std::vector<DynamicLinkModuleDescriptor> linkedDynamicModules;
		std::vector<DynamicLinkImportDescriptor> linkedDynamicImports;

		bool IsValid() const;
		void Free();
		uintptr_t GetBaseAddress() const;
	};
}