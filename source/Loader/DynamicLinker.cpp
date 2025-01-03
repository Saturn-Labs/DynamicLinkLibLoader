#include "Loader/DynamicLinker.hpp"
#include "Loader/DynamicHandle.hpp"
#include "Descriptors/DynamicImportDescriptor.hpp"
#include "Common/ArchUtils.hpp"
#include "Common/ModuleUtils.hpp"
#include "Common/Log.hpp"
#include <libhat.hpp>

namespace DynaLink {
	DynamicImportResolutionResult DynamicLinker::GetAllDynamicImports(const std::weak_ptr<DynamicHandle>& handle) {
		if (handle.expired()) {
			assert(false, "Handle was expired.");
			return DynamicImportResolutionResult::InvalidModule;
		}

		DynamicHandle& dynamicHandle = *handle.lock();
		if (!dynamicHandle.IsValid()) {
			assert(false, "Handle was invalid.");
			return DynamicImportResolutionResult::InvalidModule;
		}

		auto* dLinkSection = ModuleUtils::GetDynamicLinkSection(dynamicHandle.GetHandle());
		if (dLinkSection == nullptr) {
			return DynamicImportResolutionResult::NoDynamicImports;
		}

		uintptr_t moduleBase = dynamicHandle.GetBaseAddress();
		IMAGE_IMPORT_DESCRIPTOR* importDescriptor = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(moduleBase + dLinkSection->VirtualAddress);
		uint32_t descriptorIndex = 0;
		while (importDescriptor->Name != 0) {
			DynamicImportDescriptor::EmplaceInto(handle, dLinkSection, descriptorIndex);
			++importDescriptor;
			++descriptorIndex;
		}

		return DynamicImportResolutionResult::ResolvedAll;
	}

	DynamicLinkingResult DynamicLinker::LinkAllDynamicImports(const std::weak_ptr<DynamicHandle>& handle)
	{
		if (handle.expired()) {
			assert(false, "Handle was expired.");
			return DynamicLinkingResult::Failed;
		}

		DynamicHandle& dynamicHandle = *handle.lock();
		if (!dynamicHandle.IsValid()) {
			assert(false, "Handle was invalid.");
			return DynamicLinkingResult::Failed;
		}

		auto& dynamicImportDescriptors = dynamicHandle.GetDynamicImportDescriptors();
		for (auto& [moduleName, descriptor] : dynamicImportDescriptors) {
			HMODULE moduleHandle = GetModuleHandleA(moduleName.c_str());
			if (!ModuleUtils::IsModuleValid(moduleHandle)) {
				continue;
			}

			Semver targetVersion = ModuleUtils::GetModuleVersion(moduleHandle);
			auto& parsedDynamicImports = dynamicHandle.GetParsedDynamicImports();
			auto& dynamicImportModel = parsedDynamicImports[moduleName];
			for (auto& [symbolName, symbolDescriptor] : descriptor.GetSymbols()) {
				auto symbolIterator = std::find_if(dynamicImportModel.symbols.begin(), dynamicImportModel.symbols.end(), [&symbolName, &targetVersion](const std::pair<std::string, DynamicSymbolModel>& pair) {
					const auto& [symbol, model] = pair;
					return symbolName == symbol && targetVersion == Semver(model.version) && model.architecture == ArchUtils::GetArchitectureName();
				});

				if (symbolIterator == dynamicImportModel.symbols.end()) {
					continue;
				}

				auto& [symbol, model] = *symbolIterator;
				if (model.pointer.type == "offset") {
					uint64_t offset = std::stoull(model.pointer.value, nullptr, 16);
					uintptr_t address = reinterpret_cast<uintptr_t>(reinterpret_cast<uintptr_t>(moduleHandle) + offset);
					symbolDescriptor.WriteAddress(address);
				}
				else if (model.pointer.type == "pattern") {
					auto signature = hat::parse_signature(model.pointer.value);
					if (!signature.has_value()) {
						continue;
					}
					auto result = hat::find_pattern(signature.value(), ".text", *hat::process::get_module(moduleName));
					if (!result.has_result()) {
						continue;
					}
					symbolDescriptor.WriteAddress(reinterpret_cast<uintptr_t>(result.get()));
				}
			}
		}

		auto linkedModules = dynamicHandle.GetDynamicImportDescriptors() | std::views::filter([](const DynamicImportDescriptorMap::value_type& pair) {
			const auto& [moduleName, descriptor] = pair;
			return std::find_if(descriptor.GetSymbols().begin(), descriptor.GetSymbols().end(), [](const DynamicSymbolDescriptor& symbol) {
				return symbol.IsAddressBound();
			}) != descriptor.GetSymbols().end();
		});

		auto linkedSymbols = linkedModules | std::views::transform([](const DynamicImportDescriptorMap::value_type& pair) {
			const auto& [moduleName, descriptor] = pair;
			return descriptor.GetSymbols();
		});

		return DynamicLinkingResult::Success;
	}
}