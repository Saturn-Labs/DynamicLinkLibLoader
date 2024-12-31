#pragma once
#include <string>
#include <vector>
#include "DynamicLinkImportPointerModel.hpp"

namespace DynaLink {
	struct DynamicLinkImportModel {
		std::string symbol;
		std::vector<DynamicLinkImportPointerModel> pointers;
	};
}