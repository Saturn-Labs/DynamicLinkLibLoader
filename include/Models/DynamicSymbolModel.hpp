#pragma once
#include <string>
#include <vector>
#include "DynamicSymbolPointerModel.hpp"

namespace DynaLink {
	struct DynamicSymbolModel {
		std::string version;
		std::string architecture;
		std::string symbol;
		DynamicSymbolPointerModel pointer;
	};
}