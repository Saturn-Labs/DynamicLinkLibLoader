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

		DynamicModule(void* handle, const std::string& file,
			const std::vector<DynamicLinkModuleModel>& parsedModules,
			const std::vector<DynamicLinkModuleDescriptor>& moduleDescriptors,
			const std::vector<DynamicLinkModuleDescriptor>& linkedModules,
			const std::vector<DynamicLinkImportDescriptor>& linkedImports);

		bool IsValid() const;
		void Free();
		uintptr_t GetBaseAddress() const;

		bool Equals(const DynamicModule& other) const;
		bool operator==(const DynamicModule& other) const;
		bool operator!=(const DynamicModule& other) const;
	};
}