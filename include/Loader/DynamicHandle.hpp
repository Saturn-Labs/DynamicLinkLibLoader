#pragma once
#include "Common/Common.hpp"
#include "Enums/DynamicLinkResult.hpp"
#include "Models/DynamicImportModel.hpp"
#include "Descriptors/DynamicImportDescriptor.hpp"
#include <windows.h>

namespace DynaLink {
	struct DynamicImportModel;
	class DynamicImportDescriptor;

	using DynamicImportDescriptorMap = std::unordered_map<std::string, DynamicImportDescriptor>;
	using DynamicImportModelMap = std::unordered_map<std::string, DynamicImportModel>;

	class LOADER_API DynamicHandle {
	public:
		bool IsValid() const;
		HMODULE GetHandle() const;
		uintptr_t GetBaseAddress() const;
		std::string GetFile() const;
		std::string GetModuleName() const;
		bool HasEntryPointDelaySupport() const;
		uintptr_t GetEntryPoint() const;
		void DisableEntryPointDelayRedirection();

		const DynamicImportModelMap& GetParsedDynamicImports() const;
		DynamicImportModelMap& GetParsedDynamicImports();
		const DynamicImportDescriptorMap& GetDynamicImportDescriptors() const;
		DynamicImportDescriptorMap& GetDynamicImportDescriptors();

		DynamicLinkResult GetDynamicLinkResult() const;
		void SetDynamicLinkResult(DynamicLinkResult result);
		
		bool Equals(const DynamicHandle& other) const;
		bool operator==(const DynamicHandle& other) const;
		bool operator!=(const DynamicHandle& other) const;
		operator HMODULE() const;

		static std::shared_ptr<DynamicHandle> Create(HMODULE handle, const std::string& file, const std::vector<std::string>& dynamicLinkFiles = {});

	private:
		DynamicHandle(HMODULE handle, const std::string& file);
		DynamicHandle() = delete;
		DynamicHandle(const DynamicHandle&) = delete;
		DynamicHandle(DynamicHandle&&) = delete;
		DynamicHandle& operator=(const DynamicHandle&) = delete;
		DynamicHandle& operator=(DynamicHandle&&) = delete;

	private:
		HMODULE originalHandle = nullptr;
		std::string moduleFile = "";
		DynamicLinkResult dynamicLinkResult = DynamicLinkResult::Unlinked;
		bool isLoaded = false;

		DynamicImportModelMap parsedDynamicImports;
		DynamicImportDescriptorMap dynamicImportDescriptors;
	};
}