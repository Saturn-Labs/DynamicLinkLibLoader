#pragma once
#include "Common/Common.hpp"

namespace DynaLink {
	class LOADER_API ArchUtils {
	public:
		static bool IsArchitectureValid(const std::string& architecture);
		static std::string NormalizeArchitecture(const std::string& architecture);
		static std::string GetArchitectureName();
	};
}