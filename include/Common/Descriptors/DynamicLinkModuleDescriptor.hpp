#pragma once
#include "Common/Models/DynamicModule.hpp"

namespace DynaLink {
	struct DynamicLinkModuleDescriptor {
		DynamicModule* module = nullptr;
		IMAGE_IMPORT_DESCRIPTOR* importDescriptor = nullptr;

		bool IsValid() const {
			return module != nullptr && importDescriptor != nullptr;
		}
		std::string GetModuleName() {
			if (!IsValid()) {
				return "";
			}
			return module->handle
		}
	};
}