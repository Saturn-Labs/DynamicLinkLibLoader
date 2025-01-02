#pragma once
#include <string>
#include <vector>
#include "DynamicSymbolPointerModel.hpp"

namespace DynaLink {
	struct DynamicSymbolModel {
		std::string architecture;
		std::string symbol;
		std::vector<DynamicSymbolPointerModel> pointers;
	};
}