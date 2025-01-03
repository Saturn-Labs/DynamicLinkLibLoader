#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "DynamicSymbolPointerModel.hpp"

namespace DynaLink {
	struct DynamicSymbolModel {
		using Unordered = std::unordered_map<std::string, DynamicSymbolModel>;

		std::string version;
		std::string architecture;
		std::string symbol;
		DynamicSymbolPointerModel pointer;
	};
}