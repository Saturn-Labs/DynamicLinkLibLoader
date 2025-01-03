#include <windows.h>
#include "Loader/DynamicHandle.hpp"
#include "Loader/DynamicLinker.hpp"
#include "Common/ModuleUtils.hpp"
#include "Descriptors/DynamicImportDescriptor.hpp"
#include "Descriptors/DynamicSymbolDescriptor.hpp"
#include "Models/DynamicImportModel.hpp"
#include <ranges>

namespace DynaLink
{
	DynamicHandle::DynamicHandle(
		HMODULE handle, 
		const std::string& file) : 
		originalHandle(handle), 
		moduleFile(file),
		parsedDynamicImports({}),
		dynamicImportDescriptors({}) {}

	bool DynamicHandle::IsValid() const
	{
		return moduleFile != "" && ModuleUtils::IsModuleValid(originalHandle);
	}
	HMODULE DynamicHandle::GetHandle() const
	{
		return originalHandle;
	}
	uintptr_t DynamicHandle::GetBaseAddress() const
	{
		return reinterpret_cast<uintptr_t>(originalHandle);
	}
	std::string DynamicHandle::GetFile() const
	{
		return moduleFile;
	}
	std::string DynamicHandle::GetModuleName() const {
		return fs::path(moduleFile).filename().string();
	}

	bool DynamicHandle::HasEntryPointDelaySupport() const
	{
		if (!IsValid()) {
			return false;
		}

		IMAGE_NT_HEADERS* ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(GetBaseAddress() + reinterpret_cast<IMAGE_DOS_HEADER*>(GetBaseAddress())->e_lfanew);
		uint16_t sectionCount = ntHeaders->FileHeader.NumberOfSections;
		IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeaders);
		for (uint16_t i = 0; i < sectionCount; i++) {
			if (strcmp(reinterpret_cast<const char*>(section->Name), ".dlmre") == 0) {
				return true;
			}
			++section;
		}
		return false;
	}

	uintptr_t DynamicHandle::GetEntryPoint() const
	{
		if (!IsValid()) {
			return 0u;
		}

		IMAGE_NT_HEADERS* ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(GetBaseAddress() + reinterpret_cast<IMAGE_DOS_HEADER*>(GetBaseAddress())->e_lfanew);
		if (!HasEntryPointDelaySupport()) {
			return GetBaseAddress() + ntHeaders->OptionalHeader.AddressOfEntryPoint;
		}
		else {
			uint16_t sectionCount = ntHeaders->FileHeader.NumberOfSections;
			IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeaders);
			for (uint16_t i = 0; i < sectionCount; i++) {
				if (strcmp(reinterpret_cast<const char*>(section->Name), ".dlmre") == 0) {
					 uint32_t* dlmreDesc = reinterpret_cast<uint32_t*>(GetBaseAddress() + section->VirtualAddress);
					 if (dlmreDesc[0] == 1) {
						 return GetBaseAddress() + dlmreDesc[1];
					 }
				}
				++section;
			}
		}
		return GetBaseAddress() + ntHeaders->OptionalHeader.AddressOfEntryPoint;
	}

	void DynamicHandle::DisableEntryPointDelayRedirection()
	{
		if (!IsValid() || !HasEntryPointDelaySupport()) {
			return;
		}

		IMAGE_NT_HEADERS* ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(GetBaseAddress() + reinterpret_cast<IMAGE_DOS_HEADER*>(GetBaseAddress())->e_lfanew);
		DWORD oldProtection = 0;
		VirtualProtect(&ntHeaders->OptionalHeader.AddressOfEntryPoint, sizeof(ntHeaders->OptionalHeader.AddressOfEntryPoint), PAGE_READWRITE, &oldProtection);
		ntHeaders->OptionalHeader.AddressOfEntryPoint = GetEntryPoint() - GetBaseAddress();
		VirtualProtect(&ntHeaders->OptionalHeader.AddressOfEntryPoint, sizeof(ntHeaders->OptionalHeader.AddressOfEntryPoint), oldProtection, &oldProtection);
	}

	const DynamicImportModel::Unordered& DynamicHandle::GetParsedDynamicImports() const {
		return parsedDynamicImports;
	}
	DynamicImportModel::Unordered& DynamicHandle::GetParsedDynamicImports() {
		return parsedDynamicImports;
	}
	const DynamicImportDescriptor::Unordered& DynamicHandle::GetDynamicImportDescriptors() const
	{
		return dynamicImportDescriptors;
	}
	DynamicImportDescriptor::Unordered& DynamicHandle::GetDynamicImportDescriptors()
	{
		return dynamicImportDescriptors;
	}

	DynamicSymbolModel::Unordered DynamicHandle::GetParsedDynamicSymbols() const {
		DynamicSymbolModel::Unordered symbols;
		for (const auto& [_, import] : GetParsedDynamicImports()) {
			for (const auto& [_, symbol] : import.symbols) {
				symbols.emplace(symbol.symbol, symbol);
			}
		}
		return symbols;
	}
	DynamicSymbolDescriptor::Unordered DynamicHandle::GetDynamicSymbolDescriptors() const {
		DynamicSymbolDescriptor::Unordered symbols;
		for (const auto& [_, import] : GetDynamicImportDescriptors()) {
			for (const auto& [_, symbol] : import.GetSymbols()) {
				symbols.emplace(symbol.GetSymbol(), symbol);
			}
		}
		return symbols;
	}

	DynamicLinkResult DynamicHandle::GetDynamicLinkResult() const {
		return dynamicLinkResult;
	}

	void DynamicHandle::SetDynamicLinkResult(DynamicLinkResult result) {
		dynamicLinkResult = result;
	}

	bool DynamicHandle::Equals(const DynamicHandle& other) const {
		return IsValid() && other.IsValid() && originalHandle == other.originalHandle && moduleFile == other.moduleFile;
	}
	bool DynamicHandle::operator==(const DynamicHandle& other) const {
		return Equals(other);
	}
	bool DynamicHandle::operator!=(const DynamicHandle& other) const {
		return !Equals(other);
	}

	DynamicHandle::operator HMODULE() const {
		return originalHandle;
	}

	std::shared_ptr<DynamicHandle> DynamicHandle::Create(HMODULE handle, const std::string& file, const std::vector<std::string>& dynamicLinkFiles) {
		if (!ModuleUtils::IsModuleValid(handle) || !fs::exists(file) || !fs::is_regular_file(file)) {
			return nullptr;
		}

		auto dynamicHandle = std::shared_ptr<DynamicHandle>(new DynamicHandle(handle, file));
		for (const auto& dynamicLinkFile : dynamicLinkFiles) {
			auto model = DynamicImportModel::ParseFromFile(dynamicLinkFile);
			if (!model) {
				continue;
			}
			dynamicHandle->parsedDynamicImports.insert({ model->target, *model });
		}
		DynamicLinker::GetAllDynamicImports(dynamicHandle);
		return dynamicHandle;
	}
}