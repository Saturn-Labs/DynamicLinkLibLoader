#include "Loader/DynamicModule.hpp"
#include <windows.h>
#include "Models/DynamicLinkModuleModel.hpp"
#include "Descriptors/DynamicLinkModuleDescriptor.hpp"
#include "Loader/Loader.hpp"
#include <memory>

namespace DynaLink
{
	DynamicModule::DynamicModule(
		void* handle, 
		const std::string& file, 
		const std::vector<DynamicLinkModuleModel>& parsedModules, 
		const std::vector<DynamicLinkModuleDescriptor>& moduleDescriptors, 
		const std::vector<DynamicLinkModuleDescriptor>& linkedModules, 
		const std::vector<DynamicLinkImportDescriptor>& linkedImports) : 
		moduleHandle(handle), 
		moduleFile(file),
		parsedDynamicLinkModules(parsedModules),
		dynamicLinkModuleDescriptors(moduleDescriptors),
		linkedDynamicModules(linkedModules),
		linkedDynamicImports(linkedImports) {
	}

	bool DynamicModule::IsValid() const
	{
		return moduleHandle != nullptr && moduleFile != "" && GetModuleHandle(fs::path(moduleFile).filename().string().c_str());
	}

	void DynamicModule::Free()
	{
		if (IsValid())
		{
			FreeLibrary(reinterpret_cast<HMODULE>(moduleHandle));
			moduleHandle = nullptr;
			auto it = std::find_if(Loader::loadedDynamicModules.begin(), Loader::loadedDynamicModules.end(), [*this](const std::shared_ptr<DynamicModule>& dynModule) {
				return this->Equals(*dynModule);
			});
			if (it != Loader::loadedDynamicModules.end()) {
				Loader::loadedDynamicModules.erase(it);
			}
		}
	}

	uintptr_t DynamicModule::GetBaseAddress() const
	{
		return reinterpret_cast<uintptr_t>(moduleHandle);
	}

	bool DynamicModule::Equals(const DynamicModule& other) const {
		return moduleHandle == other.moduleHandle && moduleFile == other.moduleFile;
	}

	bool DynamicModule::operator==(const DynamicModule& other) const {
		return Equals(other);
	}

	bool DynamicModule::operator!=(const DynamicModule& other) const {
		return !Equals(other);
	}
}