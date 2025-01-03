#pragma once
#include "Common/Common.hpp"

namespace DynaLink {
	class DynamicHandle;
	enum class DynamicImportResolutionResult {
		InvalidModule,
		NoDynamicImports,
		ResolvedAll
	};

	enum class DynamicLinkingResult {
		Unknown,
		Success,
		Partial,
		Failed
	};

	class LOADER_API DynamicLinker {
	public:
		static DynamicImportResolutionResult GetAllDynamicImports(const std::weak_ptr<DynamicHandle>& handle);
		static DynamicLinkingResult LinkAllDynamicImports(const std::weak_ptr<DynamicHandle>& handle);
	};
}