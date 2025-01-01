#include "Descriptors/DynamicLinkModuleDescriptor.hpp"
#include "Loader/DynamicModule.hpp"
#include "Loader/Loader.hpp"

namespace DynaLink {
	std::optional<DynamicLinkModuleDescriptor> DynamicLinkModuleDescriptor::Create(const std::string& moduleName, void* moduleBase, const IMAGE_IMPORT_DESCRIPTOR& dynamicImportDescriptor) {
		static IMAGE_IMPORT_DESCRIPTOR zero = { 0 };
		if (moduleBase == nullptr || !Loader::IsModuleValid(reinterpret_cast<HMODULE>(moduleBase)) || memcmp(&dynamicImportDescriptor, &zero, sizeof(zero)) == 0) {
			return std::nullopt;
		}
		
		DynamicLinkModuleDescriptor descriptor = { moduleName, moduleBase, dynamicImportDescriptor, {} };
		if (dynamicImportDescriptor.FirstThunk != 0 && dynamicImportDescriptor.OriginalFirstThunk != 0 && dynamicImportDescriptor.ForwarderChain == -1) {
			uintptr_t* importLookupTable = reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(moduleBase) + dynamicImportDescriptor.OriginalFirstThunk);
			uintptr_t* importAddressTable = reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(moduleBase) + dynamicImportDescriptor.FirstThunk);

			while (*importLookupTable != 0) {
				bool isOrdinal = IMAGE_SNAP_BY_ORDINAL(*importLookupTable);
				if (isOrdinal) {
					// Skip this import, we do not support ordinal imports.
					++importLookupTable;
					++importAddressTable;
					continue;
				}
				PIMAGE_IMPORT_BY_NAME byNameImport = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(reinterpret_cast<uintptr_t>(moduleBase) + *importLookupTable);
				DynamicLinkImportDescriptor importDescriptor = { descriptor.GetImportModuleName(), byNameImport->Name, importAddressTable};
				descriptor.importDescriptors.push_back(importDescriptor);
				++importLookupTable;
				++importAddressTable;
			}
		}
		return descriptor;
	}

	bool DynamicLinkModuleDescriptor::IsValid() const {
		return moduleBase != nullptr && Loader::IsModuleValid(reinterpret_cast<HMODULE>(moduleBase)) && HasDynamicImports();
	}

	bool DynamicLinkModuleDescriptor::HasDynamicImports() const {
		static IMAGE_IMPORT_DESCRIPTOR zero = { 0 };
		return memcmp(&moduleImportDescriptor, &zero, sizeof(zero)) != 0;
	}

	std::string DynamicLinkModuleDescriptor::GetImportModuleName() const {
		return moduleName;
	}

	bool DynamicLinkModuleDescriptor::Equals(const DynamicLinkModuleDescriptor& other) const {
		if (!IsValid()) {
			return false;
		}
		return moduleBase == other.moduleBase && ((HasDynamicImports() && other.HasDynamicImports()) && memcmp(&moduleImportDescriptor, &other.moduleImportDescriptor, sizeof(moduleImportDescriptor)) == 0);
	}

	bool DynamicLinkModuleDescriptor::operator==(const DynamicLinkModuleDescriptor& other) const {
		return Equals(other);
	}

	bool DynamicLinkModuleDescriptor::operator!=(const DynamicLinkModuleDescriptor& other) const {
		return !Equals(other);
	}
}