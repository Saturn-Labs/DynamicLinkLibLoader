#pragma once
#include "Loader/DynamicModule.hpp"
#include "Common/Common.hpp"
#include <Windows.h>

namespace DynaLink {
	struct DynamicLinkModuleDescriptor;
	struct LOADER_API DynamicLinkImportDescriptor {
		std::string moduleName;
		const char* importName;

		DynamicLinkImportDescriptor(const std::string& moduleName, const char* importName, uintptr_t* importAddress);
		void WriteAddress(uintptr_t address);
		uintptr_t ReadAddress() const;
		uintptr_t* GetIATEntryAddress() const;
		bool Equals(const DynamicLinkImportDescriptor& other) const;
		bool operator==(const DynamicLinkImportDescriptor& other) const;
		bool operator!=(const DynamicLinkImportDescriptor& other) const;

	private:
		uintptr_t* importAddress;
	};
}