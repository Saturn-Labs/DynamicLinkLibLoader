#include "Loader/DynamicHandle.hpp"
#include <windows.h>
#include "Models/DynamicLinkModuleModel.hpp"
#include "Loader/DynamicLinker.hpp"
#include "Loader/Loader.hpp"
#include "Common/ModuleUtils.hpp"
#include <memory>

namespace DynaLink
{
	DynamicHandle::DynamicHandle(
		HMODULE handle, 
		const std::string& file) : 
		originalHandle(handle), 
		moduleFile(file) {
	}

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

	const DynamicLinkModuleModelMap& DynamicHandle::GetParsedDynamicLinkModules() const {
		return parsedDynamicLinkModules;
	}
	DynamicLinkModuleModelMap& DynamicHandle::GetParsedDynamicLinkModules() {
		return parsedDynamicLinkModules;
	}
	const DynamicImportDescriptorMap& DynamicHandle::GetDynamicImportDescriptors() const
	{
		return dynamicImportDescriptors;
	}
	DynamicImportDescriptorMap& DynamicHandle::GetDynamicImportDescriptors()
	{
		return dynamicImportDescriptors;
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

	std::shared_ptr<DynamicHandle> DynamicHandle::Create(HMODULE handle, const std::string& file) {
		if (!ModuleUtils::IsModuleValid(handle) || !fs::exists(file) || !fs::is_regular_file(file)) {
			return nullptr;
		}

		auto dynamicHandle = std::shared_ptr<DynamicHandle>(new DynamicHandle(handle, file));
		DynamicLinker::GetAllDynamicImports(dynamicHandle);
		return dynamicHandle;
	}
}