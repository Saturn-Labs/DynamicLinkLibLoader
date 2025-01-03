#include "Loader/DynamicLoader.hpp"
#include "Loader/DynamicHandle.hpp"
#include "Loader/DynamicLinker.hpp"
#include "Common/ModuleUtils.hpp"
#include "Common/Log.hpp"
#include <future>

namespace DynaLink {
	std::unordered_map<HMODULE, std::shared_ptr<DynamicHandle>> DynamicLoader::dynamicHandles = {};
	std::weak_ptr<DynamicHandle> DynamicLoader::LoadDynamicLinkLibrary(const std::string& moduleFile, const std::vector<std::string>& dynamicLinkFiles, DynamicLinkLoadingResult* result)
	{
		DynamicLinkLoadingResult dummmyResultValue = DynamicLinkLoadingResult::Unknown;
		if (result == nullptr) {
			result = &dummmyResultValue;
		}

		if (moduleFile.empty()) {
			LOG_ERROR("Failed to load module, invalid file name '{}'!", moduleFile);
			*result = DynamicLinkLoadingResult::InvalidModule;
			return {};
		}

		std::string moduleName = fs::path(moduleFile).filename().string();
		if (moduleName.empty()) {
			LOG_ERROR("Failed to load module '{}', invalid module name!", moduleFile);
			*result = DynamicLinkLoadingResult::InvalidModule;
			return {};
		}

		std::string fixedModulePath;
		if (!ModuleUtils::FindModuleOnStandardPaths(moduleFile, &fixedModulePath)) {
			LOG_ERROR("Failed to load module '{}', file not found!", moduleName);
			return {};
		}

		HMODULE moduleHandle = GetModuleHandleA(moduleFile.c_str());
		if (moduleHandle != nullptr) {
			if (!ModuleUtils::GetDynamicLinkSection(moduleHandle)) {
				LOG_ERROR("Module '{}' is not a dynamic link module!", moduleName);
				*result = DynamicLinkLoadingResult::NotDynaLinkModule;
				return {};
			}

			if (dynamicHandles.contains(moduleHandle)) {
				LOG_WARN("Module '{}' found on the dynamic link module repository, returning.", moduleName);
				*result = DynamicLinkLoadingResult::AlreadyLoaded;
				return dynamicHandles[moduleHandle];
			}

			LOG_WARN("Module '{}' module could not be found on the dynamic link module repository, reloading it.", moduleName);
			std::shared_ptr<DynamicHandle> moduleResult = DynamicHandle::Create(LoadLibraryA(moduleFile.c_str()), fixedModulePath, dynamicLinkFiles);
			dynamicHandles[moduleHandle] = moduleResult;
			DynamicLinker::LinkAllDynamicImports(moduleResult);
			*result = DynamicLinkLoadingResult::ModuleReloaded;
			return moduleResult;
		}

		if (!ModuleUtils::IsDynaLinkModuleFile(moduleFile.c_str())) {
			LOG_ERROR("Module '{}' is not a dynamic link module!", moduleName);
			*result = DynamicLinkLoadingResult::NotDynaLinkModule;
			return {};
		}

		LOG_TRACE("Loading module '{}'...", moduleName);
		moduleHandle = LoadLibraryA(moduleFile.c_str());
		if (moduleHandle == nullptr) {
			DWORD lastError = GetLastError();
			char* errorMessage = nullptr;
			FormatMessageA(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr,
				lastError,
				0,
				(LPSTR)&errorMessage,
				0,
				nullptr
			);
			LOG_ERROR("Failed to load module '{}', windows internal error 0x{:x} '{}'...", moduleName, lastError, const_cast<const char*>(errorMessage));
			return {};
		}

		std::shared_ptr<DynamicHandle> moduleResult = DynamicHandle::Create(moduleHandle, fixedModulePath, dynamicLinkFiles);
		dynamicHandles[moduleHandle] = moduleResult;
		DynamicLinker::LinkAllDynamicImports(moduleResult);
		if (moduleResult->HasEntryPointDelaySupport()) {
			auto dllMain = reinterpret_cast<BOOL(APIENTRY*)(HMODULE, DWORD, LPVOID)>(moduleResult->GetEntryPoint());
			moduleResult->DisableEntryPointDelayRedirection();
			std::async(std::launch::async, [&moduleResult, dllMain]() {
				BOOL result = dllMain(moduleResult->GetHandle(), DLL_PROCESS_ATTACH, nullptr);
				if (result == FALSE) {
					FreeLibraryAndExitThread(moduleResult->GetHandle(), moduleResult ? 1 : 0);
				}
			});
		}
		*result = DynamicLinkLoadingResult::ModuleLoaded;
		return moduleResult;
	}
}

HMODULE WINAPI LoadDynamicLinkLibrary(const std::string& moduleFile, const std::vector<std::string>& dynamicLinkFiles, DynaLink::DynamicLinkLoadingResult* result) {
	auto handle = DynaLink::DynamicLoader::LoadDynamicLinkLibrary(moduleFile, dynamicLinkFiles, result);
	if (handle.expired()) {
		return nullptr;
	}
	return handle.lock()->GetHandle();
}