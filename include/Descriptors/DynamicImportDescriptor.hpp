#pragma once
#include "Common/Common.hpp"
#include <windows.h>

namespace DynaLink {
	class DynamicHandle;
	class DynamicSymbolDescriptor;
	class LOADER_API DynamicImportDescriptor {
	public:
		using SymbolMap = std::unordered_map<std::string, DynamicSymbolDescriptor>;

		static std::optional<DynamicImportDescriptor&> EmplaceInto(const std::weak_ptr<DynamicHandle>& handle, IMAGE_SECTION_HEADER* dlinkSection, size_t indexOfDescriptor);

		bool IsValid() const;
		operator bool() const;

		const SymbolMap& GetSymbols() const;
		SymbolMap& GetSymbols();

		SymbolMap::const_iterator begin() const;
		SymbolMap::const_iterator end() const;
		SymbolMap::iterator begin();
		SymbolMap::iterator end();
		const DynamicSymbolDescriptor& operator[](const std::string& symbol) const;
		DynamicSymbolDescriptor& operator[](const std::string& symbol);

	private:
		DynamicImportDescriptor(const std::weak_ptr<DynamicHandle>& handle, IMAGE_IMPORT_DESCRIPTOR* importDescriptor, IMAGE_SECTION_HEADER* dlinkSection, size_t myIndex, size_t symbolCount, const std::string& moduleName);
		DynamicImportDescriptor() = delete;
		DynamicImportDescriptor(const DynamicImportDescriptor&) = delete;
		DynamicImportDescriptor(DynamicImportDescriptor&&) = delete;
		DynamicImportDescriptor& operator=(const DynamicImportDescriptor&) = delete;
		DynamicImportDescriptor& operator=(DynamicImportDescriptor&&) = delete;

	private:
		friend class DynamicHandle;
		friend class DynamicSymbolDescriptor;

		std::weak_ptr<DynamicHandle> handle = {};
		IMAGE_IMPORT_DESCRIPTOR* peImportDescriptor = nullptr;
		IMAGE_SECTION_HEADER* dlinkSection = nullptr;
		size_t myIndex = 0;
		size_t symbolCount = 0;
		std::string moduleName = "";
		SymbolMap symbols = {};
	};
}