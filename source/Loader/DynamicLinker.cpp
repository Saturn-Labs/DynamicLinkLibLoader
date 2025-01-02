#include "Loader/DynamicLinker.hpp"
#include "Loader/DynamicHandle.hpp"
#include "Descriptors/DynamicImportDescriptor.hpp"
#include "Common/ModuleUtils.hpp"

namespace DynaLink {
	DynamicImportResolutionResult DynamicLinker::GetAllDynamicImports(const std::weak_ptr<DynamicHandle>& handle) {
		if (handle.expired()) {
			assert(false, "Handle was expired.");
			return DynamicImportResolutionResult::InvalidModule;
		}

		DynamicHandle& dynamicHandle = *handle.lock();
		if (!dynamicHandle.IsValid()) {
			assert(false, "Handle was invalid.");
			return DynamicImportResolutionResult::InvalidModule;
		}

		auto* dLinkSection = ModuleUtils::GetDynamicLinkSection(dynamicHandle.GetHandle());
		if (dLinkSection == nullptr) {
			return DynamicImportResolutionResult::NoDynamicImports;
		}

		uintptr_t moduleBase = dynamicHandle.GetBaseAddress();
		IMAGE_IMPORT_DESCRIPTOR* importDescriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(moduleBase + dLinkSection->VirtualAddress);
		uint32_t descriptorIndex = 0;
		while (importDescriptor->Name != 0) {
			DynamicImportDescriptor::EmplaceInto(handle, dLinkSection, descriptorIndex);
			++importDescriptor;
			++descriptorIndex;
		}

		return DynamicImportResolutionResult::ResolvedAll;
	}
}