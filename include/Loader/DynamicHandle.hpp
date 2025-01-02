#pragma once
#include "Common/Common.hpp"
#include "Enums/DynamicLinkResult.hpp"
#include <windows.h>

namespace DynaLink {
	struct DynamicLinkModuleModel;
	struct DynamicImportDescriptor;

	using DynamicImportDescriptorMap = std::unordered_map<std::string, DynamicImportDescriptor>;
	using DynamicLinkModuleModelMap = std::unordered_map<std::string, DynamicLinkModuleModel>;

	class LOADER_API DynamicHandle {
	public:
		bool IsValid() const;
		HMODULE GetHandle() const;
		uintptr_t GetBaseAddress() const;
		std::string GetFile() const;
		std::string GetModuleName() const;

		const DynamicLinkModuleModelMap& GetParsedDynamicLinkModules() const;
		DynamicLinkModuleModelMap& GetParsedDynamicLinkModules();
		const DynamicImportDescriptorMap& GetDynamicImportDescriptors() const;
		DynamicImportDescriptorMap& GetDynamicImportDescriptors();

		DynamicLinkResult GetDynamicLinkResult() const;
		void SetDynamicLinkResult(DynamicLinkResult result);
		
		bool Equals(const DynamicHandle& other) const;
		bool operator==(const DynamicHandle& other) const;
		bool operator!=(const DynamicHandle& other) const;
		operator HMODULE() const;

		static std::shared_ptr<DynamicHandle> Create(HMODULE handle, const std::string& file);

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

		DynamicLinkModuleModelMap parsedDynamicLinkModules = {};
		DynamicImportDescriptorMap dynamicImportDescriptors = {};
	};
}