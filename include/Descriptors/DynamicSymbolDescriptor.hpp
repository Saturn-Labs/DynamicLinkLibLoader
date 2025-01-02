#pragma once
#include "Common/Common.hpp"
#include <windows.h>

namespace DynaLink {
	class DynamicImportDescriptor;
	class LOADER_API DynamicSymbolDescriptor {
	public:
		static std::optional<std::reference_wrapper<DynamicSymbolDescriptor>> EmplaceInto(DynamicImportDescriptor& importDescriptor, size_t entryIndex);

		bool IsValid() const;
		operator bool() const;

		std::string GetSymbol() const;

		uintptr_t ReadAddress() const;
		void WriteAddress(uintptr_t address) const;
		bool IsAddressBound() const;

		uintptr_t* GetImportLookupTableEntry() const;
		uintptr_t* GetImportAddressTableEntry() const;

		void ProtectImportAddressTableEntry(DWORD protection) const;

		bool Equals(const DynamicSymbolDescriptor& other) const;
		bool operator==(const DynamicSymbolDescriptor& other) const;
		bool operator!=(const DynamicSymbolDescriptor& other) const;
		DynamicSymbolDescriptor(
			DynamicImportDescriptor& importDescriptor,
			uintptr_t* importLookupTableEntry,
			uintptr_t* importAddressTableEntry,
			IMAGE_IMPORT_BY_NAME* importByName,
			const std::string& symbol
		);

	private:
		DynamicSymbolDescriptor() = delete;

	private:
		friend class DynamicHandle;
		friend class DynamicImportDescriptor;
		friend class std::unordered_map<std::string, DynamicSymbolDescriptor>;

		DynamicImportDescriptor* importDescriptor;
		uintptr_t* importLookupTableEntry = nullptr;
		uintptr_t* importAddressTableEntry = nullptr;
		IMAGE_IMPORT_BY_NAME* importByName = nullptr;
		std::string symbol;
	};
}