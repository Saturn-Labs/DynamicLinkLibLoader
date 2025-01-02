#pragma once

namespace DynaLink {
	enum class DynamicLinkResult {
		Unlinked = -1,
		Error = 0,
		Full = 1,
		Partial = 2,
		NotDynaLinkModule = 3,
		InvalidModule = 4
	};
}