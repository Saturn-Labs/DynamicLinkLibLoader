#include "Common/Loader/DynamicModule.hpp"
#include <windows.h>
#include "Common/Models/DynamicLinkModuleModel.hpp"
#include "Common/Descriptors/DynamicLinkModuleDescriptor.hpp"

namespace DynaLink
{
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
		}
	}

	uintptr_t DynamicModule::GetBaseAddress() const
	{
		return reinterpret_cast<uintptr_t>(moduleHandle);
	}
}