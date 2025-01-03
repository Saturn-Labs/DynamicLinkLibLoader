#include "Descriptors/DynamicSymbolDescriptor.hpp"
#include "Descriptors/DynamicImportDescriptor.hpp"
#include "Loader/DynamicHandle.hpp"

namespace DynaLink {
	DynamicSymbolDescriptor::DynamicSymbolDescriptor(
		DynamicImportDescriptor& importDescriptor, 
		uintptr_t* importLookupTableEntry, 
		uintptr_t* importAddressTableEntry, 
		IMAGE_IMPORT_BY_NAME* importByName, 
		const std::string& symbol) :
		importDescriptor(&importDescriptor),
		importLookupTableEntry(importLookupTableEntry),
		importAddressTableEntry(importAddressTableEntry),
		importByName(importByName),
		symbol(symbol) {}

	std::optional<std::reference_wrapper<DynamicSymbolDescriptor>> DynamicSymbolDescriptor::EmplaceInto(DynamicImportDescriptor& importDescriptor, size_t entryIndex) {
		if (!importDescriptor.IsValid()) {
			assert(false, "Import descriptor is not valid.");
			return std::nullopt;
		}

		if (entryIndex >= importDescriptor.symbolCount) {
			assert(false, "Entry index is out of bounds.");
			return std::nullopt;
		}

		DynamicHandle& handle = *importDescriptor.handle.lock();
		uintptr_t* importLookupTableEntry = reinterpret_cast<uintptr_t*>(handle.GetBaseAddress() + importDescriptor.peImportDescriptor->OriginalFirstThunk) + entryIndex;
		uintptr_t* importAddressTableEntry = reinterpret_cast<uintptr_t*>(handle.GetBaseAddress() + importDescriptor.peImportDescriptor->FirstThunk) + entryIndex;

		if ((*importLookupTableEntry & IMAGE_ORDINAL_FLAG)) {
			return std::nullopt;
		}

		IMAGE_IMPORT_BY_NAME* importByName = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(handle.GetBaseAddress() + *importLookupTableEntry);
		return importDescriptor.symbols.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(importByName->Name),
			std::forward_as_tuple(
				importDescriptor,
				importLookupTableEntry,
				importAddressTableEntry,
				importByName,
				importByName->Name
			)
		).first->second;
	}

	bool DynamicSymbolDescriptor::IsValid() const
	{
		return importDescriptor->IsValid();
	}

	DynamicSymbolDescriptor::operator bool() const
	{
		return IsValid();
	}

	std::string DynamicSymbolDescriptor::GetSymbol() const
	{
		return symbol;
	}

	uintptr_t DynamicSymbolDescriptor::ReadAddress() const
	{
		return *importAddressTableEntry;
	}

	void DynamicSymbolDescriptor::WriteAddress(uintptr_t address) const
	{
		ProtectImportAddressTableEntry(PAGE_EXECUTE_READWRITE);
		*importAddressTableEntry = address;
		ProtectImportAddressTableEntry(PAGE_EXECUTE_READ);
	}

	bool DynamicSymbolDescriptor::IsAddressBound() const
	{
		if (*importAddressTableEntry == NULL) {
			return false;
		}

		MEMORY_BASIC_INFORMATION memInfo;
		if (!VirtualQuery(reinterpret_cast<LPCVOID>(*importAddressTableEntry), &memInfo, sizeof(memInfo))) {
			return false;
		}

		return memInfo.State == MEM_COMMIT && memInfo.Protect != PAGE_NOACCESS;
	}

	uintptr_t* DynamicSymbolDescriptor::GetImportLookupTableEntry() const
	{
		return importLookupTableEntry;
	}

	uintptr_t* DynamicSymbolDescriptor::GetImportAddressTableEntry() const
	{
		return importAddressTableEntry;
	}

	void DynamicSymbolDescriptor::ProtectImportAddressTableEntry(DWORD protection) const
	{
		if (!VirtualProtect(importAddressTableEntry, sizeof(uintptr_t), protection, nullptr)) {
			DWORD lastError = GetLastError();
			char* errorMessage = nullptr;
			FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				lastError,
				0,
				(LPSTR)&errorMessage,
				0,
				nullptr
			);

			// TODO: Fix
			assert(false, "Failed to protect import address table entry. {}", errorMessage);
		}
	}

	bool DynamicSymbolDescriptor::Equals(const DynamicSymbolDescriptor& other) const
	{
		if (!IsValid() || !other.IsValid()) {
			return false;
		}

		return 
			importDescriptor == other.importDescriptor && 
			symbol == other.symbol && 
			importLookupTableEntry == other.importLookupTableEntry && 
			importAddressTableEntry == other.importAddressTableEntry;
	}

	bool DynamicSymbolDescriptor::operator==(const DynamicSymbolDescriptor& other) const
	{
		return Equals(other);
	}

	bool DynamicSymbolDescriptor::operator!=(const DynamicSymbolDescriptor& other) const
	{
		return !Equals(other);
	}
}