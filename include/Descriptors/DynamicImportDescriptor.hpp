#pragma once
#include "Descriptors/DynamicSymbolDescriptor.hpp"
#include "Common/Common.hpp"
#include <windows.h>

namespace DynaLink {
	class DynamicHandle;
	class DynamicSymbolDescriptor;
	class LOADER_API DynamicImportDescriptor {
	public:
		using Unordered = std::unordered_map<std::string, DynamicImportDescriptor>;

		static std::optional<std::reference_wrapper<DynamicImportDescriptor>> EmplaceInto(const std::weak_ptr<DynamicHandle>& handle, IMAGE_SECTION_HEADER* dlinkSection, size_t indexOfDescriptor);

		bool IsValid() const;
		operator bool() const;

		const DynamicSymbolDescriptor::Unordered& GetSymbols() const;
		DynamicSymbolDescriptor::Unordered& GetSymbols();

		DynamicSymbolDescriptor::Unordered::const_iterator begin() const;
		DynamicSymbolDescriptor::Unordered::const_iterator end() const;
		DynamicSymbolDescriptor::Unordered::iterator begin();
		DynamicSymbolDescriptor::Unordered::iterator end();
		const DynamicSymbolDescriptor& operator[](const std::string& symbol) const;
		DynamicSymbolDescriptor& operator[](const std::string& symbol);
		DynamicImportDescriptor(const std::weak_ptr<DynamicHandle>& handle, IMAGE_IMPORT_DESCRIPTOR* importDescriptor, IMAGE_SECTION_HEADER* dlinkSection, size_t myIndex, size_t symbolCount, const std::string& moduleName);

	private:
		DynamicImportDescriptor() = delete;

	private:
		friend class DynamicHandle;
		friend class DynamicSymbolDescriptor;

		std::weak_ptr<DynamicHandle> handle = {};
		IMAGE_IMPORT_DESCRIPTOR* peImportDescriptor = nullptr;
		IMAGE_SECTION_HEADER* dlinkSection = nullptr;
		size_t myIndex = 0;
		size_t symbolCount = 0;
		std::string moduleName = "";
		DynamicSymbolDescriptor::Unordered symbols;
	};
}