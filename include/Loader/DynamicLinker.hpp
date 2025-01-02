#pragma once
#include "Common/Common.hpp"

namespace DynaLink {
	class DynamicHandle;
	enum class DynamicImportResolutionResult {
		InvalidModule,
		NoDynamicImports,
		ResolvedAll
	};

	class LOADER_API DynamicLinker {
	public:
		static DynamicImportResolutionResult GetAllDynamicImports(const std::weak_ptr<DynamicHandle>& handle);
	};
}