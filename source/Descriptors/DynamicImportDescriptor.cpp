#include "Descriptors/DynamicImportDescriptor.hpp"
#include "Descriptors/DynamicSymbolDescriptor.hpp"
#include "Loader/DynamicHandle.hpp"

namespace DynaLink {
	DynamicImportDescriptor::DynamicImportDescriptor(
		const std::weak_ptr<DynamicHandle>& handle, 
		IMAGE_IMPORT_DESCRIPTOR* importDescriptor, 
		IMAGE_SECTION_HEADER* dlinkSection,
		size_t myIndex, 
		size_t symbolCount, 
		const std::string& moduleName) :
		handle(handle),
		peImportDescriptor(importDescriptor),
		dlinkSection(dlinkSection),
		myIndex(myIndex),
		symbolCount(symbolCount),
		moduleName(moduleName) {}

	std::optional<DynamicImportDescriptor&> DynamicImportDescriptor::EmplaceInto(const std::weak_ptr<DynamicHandle>& handle, IMAGE_SECTION_HEADER* dlinkSection, size_t indexOfDescriptor)
	{
		if (handle.expired()) {
			assert(false, "Handle was expired.");
			return std::nullopt;
		}

		if (dlinkSection == nullptr) {
			assert(false, "Dynamic Link Section was nullptr.");
			return std::nullopt;
		}

		DynamicHandle& dynamicHandle = *handle.lock();
		if (!dynamicHandle.IsValid()) {
			assert(false, "Handle was invalid.");
			return std::nullopt;
		}

		uintptr_t moduleBase = dynamicHandle.GetBaseAddress();
		IMAGE_IMPORT_DESCRIPTOR* importDescriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(moduleBase + dlinkSection->VirtualAddress) + indexOfDescriptor;
		uintptr_t* ilt = reinterpret_cast<uintptr_t*>(moduleBase + importDescriptor->OriginalFirstThunk);
		uint32_t symbolCount = 0;
		while ((++ilt) != 0) {
			++symbolCount;
		}
		std::string moduleName = reinterpret_cast<const char*>(moduleBase + importDescriptor->Name);
		auto& descriptor = dynamicHandle.GetDynamicImportDescriptors().emplace(
			std::piecewise_construct,
			std::forward_as_tuple(moduleName),
			std::forward_as_tuple(
				handle, 
				importDescriptor,
				dlinkSection,
				indexOfDescriptor,
				symbolCount,
				moduleName
			)
		).first->second;

		for (size_t i = 0; i < symbolCount; ++i) {
			if (!DynamicSymbolDescriptor::EmplaceInto(descriptor, i)) {
				assert(false, "Failed to emplace symbol into descriptor.");
			}
		}

		return descriptor;
	}

	bool DynamicImportDescriptor::IsValid() const
	{
		return
			!handle.expired() &&
			handle.lock()->IsValid() &&
			peImportDescriptor != nullptr &&
			!moduleName.empty();
	}

	DynamicImportDescriptor::operator bool() const
	{
		return IsValid();
	}

	const DynamicImportDescriptor::SymbolMap& DynamicImportDescriptor::GetSymbols() const
	{
		return symbols;
	}

	DynamicImportDescriptor::SymbolMap& DynamicImportDescriptor::GetSymbols()
	{
		return symbols;
	}

	DynamicImportDescriptor::SymbolMap::const_iterator DynamicImportDescriptor::begin() const
	{
		return symbols.begin();
	}

	DynamicImportDescriptor::SymbolMap::const_iterator DynamicImportDescriptor::end() const
	{
		return symbols.end();
	}

	DynamicImportDescriptor::SymbolMap::iterator DynamicImportDescriptor::begin()
	{
		return symbols.begin();
	}

	DynamicImportDescriptor::SymbolMap::iterator DynamicImportDescriptor::end()
	{
		return symbols.end();
	}

	const DynamicSymbolDescriptor& DynamicImportDescriptor::operator[](const std::string& symbol) const
	{
		return symbols.find(symbol)->second;
	}

	DynamicSymbolDescriptor& DynamicImportDescriptor::operator[](const std::string& symbol)
	{
		return symbols.find(symbol)->second;
	}
}