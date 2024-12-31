#include "Common/Descriptors/DynamicLinkModuleDescriptor.hpp"
#include "Common/Loader/DynamicModule.hpp"

namespace DynaLink {
	std::optional<DynamicLinkModuleDescriptor> DynamicLinkModuleDescriptor::Create(DynamicModule& module, const IMAGE_IMPORT_DESCRIPTOR& dynamicImportDescriptor) {
		static IMAGE_IMPORT_DESCRIPTOR zero = { 0 };
		if (!module.IsValid() || memcmp(&dynamicImportDescriptor, &zero, sizeof(zero)) == 0) {
			return std::nullopt;
		}
		
		DynamicLinkModuleDescriptor descriptor = { module, dynamicImportDescriptor, {} };
		if (dynamicImportDescriptor.FirstThunk != 0 && dynamicImportDescriptor.OriginalFirstThunk != 0 && dynamicImportDescriptor.ForwarderChain == -1) {
			uintptr_t* importLookupTable = reinterpret_cast<uintptr_t*>(module.GetBaseAddress() + dynamicImportDescriptor.OriginalFirstThunk);
			uintptr_t* importAddressTable = reinterpret_cast<uintptr_t*>(module.GetBaseAddress() + dynamicImportDescriptor.FirstThunk);

			while (*importLookupTable != 0) {
				bool isOrdinal = IMAGE_SNAP_BY_ORDINAL(*importLookupTable);
				if (isOrdinal) {
					// Skip this import, we do not support ordinal imports.
					++importLookupTable;
					++importAddressTable;
					continue;
				}
				PIMAGE_IMPORT_BY_NAME byNameImport = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(module.GetBaseAddress() + *importLookupTable);
				DynamicLinkImportDescriptor importDescriptor = { descriptor, byNameImport->Name, importAddressTable };
				descriptor.importDescriptors.push_back(importDescriptor);
				++importLookupTable;
				++importAddressTable;
			}
		}
		return descriptor;
	}

	bool DynamicLinkModuleDescriptor::IsValid() const {
		return module.IsValid() && HasDynamicImports();
	}

	bool DynamicLinkModuleDescriptor::HasDynamicImports() const {
		static IMAGE_IMPORT_DESCRIPTOR zero = { 0 };
		return memcmp(&moduleImportDescriptor, &zero, sizeof(zero)) != 0;
	}

	const char* DynamicLinkModuleDescriptor::GetImportModuleName() const {
		if (!IsValid()) {
			return "";
		}
		return reinterpret_cast<const char*>(module.GetBaseAddress() + moduleImportDescriptor.Name);
	}

	bool DynamicLinkModuleDescriptor::Equals(const DynamicLinkModuleDescriptor& other) const {
		if (!IsValid()) {
			return false;
		}
		return module.GetBaseAddress() == other.module.GetBaseAddress() && ((HasDynamicImports() && other.HasDynamicImports()) && memcmp(&moduleImportDescriptor, &other.moduleImportDescriptor, sizeof(moduleImportDescriptor)) == 0);
	}

	bool DynamicLinkModuleDescriptor::operator==(const DynamicLinkModuleDescriptor& other) const {
		return Equals(other);
	}

	bool DynamicLinkModuleDescriptor::operator!=(const DynamicLinkModuleDescriptor& other) const {
		return !Equals(other);
	}
}