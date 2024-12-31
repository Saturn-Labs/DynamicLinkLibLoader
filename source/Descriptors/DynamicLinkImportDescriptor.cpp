#include "Descriptors/DynamicLinkImportDescriptor.hpp"

namespace DynaLink {
	DynamicLinkImportDescriptor::DynamicLinkImportDescriptor(DynamicLinkModuleDescriptor& moduleDescriptor, const char* importName, uintptr_t* importAddress) : 
		moduleDescriptor(moduleDescriptor), 
		importName(importName), 
		importAddress(importAddress)
	{
	}

	void DynamicLinkImportDescriptor::WriteAddress(uintptr_t address) {
		DWORD oldProtection = 0;
		VirtualProtect(importAddress, sizeof(uintptr_t), PAGE_READWRITE, &oldProtection);
		*importAddress = address;
		VirtualProtect(importAddress, sizeof(uintptr_t), oldProtection, nullptr);
	}

	uintptr_t DynamicLinkImportDescriptor::ReadAddress() const {
		return *importAddress;
	}

	uintptr_t* DynamicLinkImportDescriptor::GetIATEntryAddress() const {
		return importAddress;
	}

	bool DynamicLinkImportDescriptor::Equals(const DynamicLinkImportDescriptor& other) const {
		return importName == other.importName && importAddress == other.importAddress;
	}

	bool DynamicLinkImportDescriptor::operator==(const DynamicLinkImportDescriptor& other) const {
		return Equals(other);
	}

	bool DynamicLinkImportDescriptor::operator!=(const DynamicLinkImportDescriptor& other) const {
		return !Equals(other);
	}
}