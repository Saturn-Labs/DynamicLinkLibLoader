#include "Common/ArchUtils.hpp"

namespace DynaLink {
	bool ArchUtils::IsArchitectureValid(const std::string& architecture)
	{
		return architecture == "amd64" || architecture == "x64" || architecture == "x86_64" || architecture == "x86" || architecture == "i386";
	}

	std::string ArchUtils::NormalizeArchitecture(const std::string& architecture)
	{
		if (!IsArchitectureValid(architecture)) {
			return "Unknown";
		}

		if (architecture == "amd64" || architecture == "x64" || architecture == "x86_64") {
			return "amd64";
		}
		else if (architecture == "x86" || architecture == "i386") {
			return "i386";
		}
	}

	std::string ArchUtils::GetArchitectureName()
	{
		if (sizeof(void*) == 8) {
			return "amd64";
		}
		else {
			return "i386";
		}
	}
}