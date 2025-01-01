#include "Loader/Loader.hpp"
#include "Loader/DynamicModule.hpp"
#include "Common/Log.hpp"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <fstream>
#pragma comment(lib, "version.lib")

namespace json = rapidjson;

namespace DynaLink {
	std::vector<std::shared_ptr<DynamicModule>> Loader::loadedDynamicModules{};

	std::vector<DynamicLinkModuleDescriptor> Loader::GetDynamicLinkModuleDescriptors(DynamicModule& dynamicModule) {
		static IMAGE_IMPORT_DESCRIPTOR zero = { 0 };
		if (!dynamicModule.IsValid()) {
			return {};
		}

		uintptr_t moduleBase = dynamicModule.GetBaseAddress();
		PIMAGE_NT_HEADERS ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(moduleBase + reinterpret_cast<PIMAGE_DOS_HEADER>(moduleBase)->e_lfanew);
		PIMAGE_SECTION_HEADER sectionHeader = IMAGE_FIRST_SECTION(ntHeaders);
		IMAGE_FILE_HEADER fileHeader = ntHeaders->FileHeader;
		IMAGE_OPTIONAL_HEADER optionalHeader = ntHeaders->OptionalHeader;
		PIMAGE_IMPORT_DESCRIPTOR dynamicImport = nullptr;
		uint16_t sectionIndex = 0;
		while (sectionIndex < fileHeader.NumberOfSections) {
			if (strcmp(reinterpret_cast<const char*>(sectionHeader->Name), ".dlink") == 0) {
				dynamicImport = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(moduleBase + sectionHeader->VirtualAddress);
				break;
			}
			++sectionIndex;
			++sectionHeader;
		}

		if (dynamicImport == nullptr) {
			return {};
		}

		while (memcmp(&dynamicImport, &zero, sizeof(zero)) != 0) {
			auto descriptor = DynamicLinkModuleDescriptor::Create(fs::path(dynamicModule.moduleFile).filename().string(), dynamicModule.moduleHandle, *dynamicImport);
			if (descriptor.has_value()) {
				dynamicModule.dynamicLinkModuleDescriptors.push_back(descriptor.value());
			}
			++dynamicImport;
		}
		return dynamicModule.dynamicLinkModuleDescriptors;
	}

	Semver Loader::GetModuleVersion(HMODULE moduleHandle) {
		static Semver any("*");
		char path[MAX_PATH];
		if (!GetModuleFileNameA(moduleHandle, path, MAX_PATH)) {
			return any;
		}

		DWORD versionHandle = 0;
		DWORD size = GetFileVersionInfoSizeA(path, &versionHandle);
		if (size == 0) {
			return any;
		}

		char* versionData = new char[size];
		if (!GetFileVersionInfoA(path, versionHandle, size, versionData)) {
			delete[] versionData;
			return any;
		}

		VS_FIXEDFILEINFO* fileInfo;
		UINT fileInfoSize;

		if (!VerQueryValueA(versionData, "\\", (LPVOID*)&fileInfo, &fileInfoSize)) {
			delete[] versionData;
			return any;
		}

		DWORD major = HIWORD(fileInfo->dwFileVersionMS);
		DWORD minor = LOWORD(fileInfo->dwFileVersionMS);
		DWORD build = HIWORD(fileInfo->dwFileVersionLS);
		DWORD revision = LOWORD(fileInfo->dwFileVersionLS);
		return Semver(fmt::format("{}.{}.{}.{}", major, minor, build, revision));
	}

	bool Loader::DynamicallyLinkModule(DynamicModule& dynamicModule) {
		for (auto& descriptor : dynamicModule.dynamicLinkModuleDescriptors) {
			if (!descriptor.IsValid()) {
				continue;
			}

			HMODULE importModule = GetModuleHandleA(descriptor.GetImportModuleName().c_str());
			if (importModule == nullptr) {
				LOG_ERROR("Failed to link module '{}' functions, required by '{}'... Module not found!", descriptor.GetImportModuleName(), dynamicModule.moduleFile);
				continue;
			}

			Semver importModuleVersion = GetModuleVersion(importModule);
			auto moduleModelIterator = std::find_if(dynamicModule.parsedDynamicLinkModules.begin(), dynamicModule.parsedDynamicLinkModules.end(), [&descriptor](const DynamicLinkModuleModel& model) {
				return model.target == descriptor.GetImportModuleName();
			});

			if (moduleModelIterator == dynamicModule.parsedDynamicLinkModules.end()) {
				LOG_ERROR("Failed to link module '{}' functions, required by '{}'... Dynamic Linking file not found!", descriptor.GetImportModuleName(), dynamicModule.moduleFile);
				continue;
			}

			DynamicLinkModuleModel moduleModel = *moduleModelIterator;
			for (auto& import : descriptor.importDescriptors) {
				auto importModelIterator = std::find_if(moduleModel.imports.begin(), moduleModel.imports.end(), [&import](const DynamicLinkImportModel& model) {
					return model.symbol == import.importName;
				});

				if (importModelIterator == moduleModel.imports.end()) {
					continue;
				}

				DynamicLinkImportModel importModel = *importModelIterator;
				for (const auto& pointer : importModel.pointers) {
					Semver pointerVerion{ pointer.version };
					if (pointerVerion.fullWildcard() || pointerVerion == importModuleVersion) {
						if (pointer.type == "offset") {
							uintptr_t targetAddress = reinterpret_cast<uintptr_t>(importModule) + std::stoul(pointer.value, nullptr, 16);
							import.WriteAddress(targetAddress);
							dynamicModule.linkedDynamicImports.push_back(import);
						}
						else if (pointer.type == "pattern") {
							LOG_WARN("Failed to link module '{}' function '{}', required by '{}'... Pattern linking not supported!", descriptor.GetImportModuleName(), import.importName, dynamicModule.moduleFile);
						}
						break;
					}
				}
			}
			dynamicModule.linkedDynamicModules.push_back(descriptor);
		}
		return (dynamicModule.dynamicLinkModuleDescriptors.size() >= 1) ? dynamicModule.linkedDynamicModules.size() == dynamicModule.dynamicLinkModuleDescriptors.size() : true;
	}

	std::vector<std::string> Loader::GetDllSearchPaths() {
		std::vector<std::string> searchPaths;
		char currentProcessDir[MAX_PATH];
		if (GetModuleFileNameA(NULL, currentProcessDir, MAX_PATH) > 0) {
			std::string processDir(currentProcessDir);
			size_t pos = processDir.find_last_of("\\/");
			if (pos != std::string::npos) {
				searchPaths.push_back(processDir.substr(0, pos));
			}
		}

		char currentDir[MAX_PATH];
		if (GetCurrentDirectoryA(MAX_PATH, currentDir) > 0) {
			searchPaths.push_back(currentDir);
		}

		// Get system directories
		char systemDir[MAX_PATH];
		if (GetSystemDirectoryA(systemDir, MAX_PATH) > 0) {
			searchPaths.push_back(systemDir);
		}

		// Get the Windows directory
		char windowsDir[MAX_PATH];
		if (GetWindowsDirectoryA(windowsDir, MAX_PATH) > 0) {
			searchPaths.push_back(windowsDir);
		}

		const char* pathEnv = std::getenv("PATH");
		if (pathEnv != nullptr) {
			std::string pathString(pathEnv);
			size_t startPos = 0, endPos;
			while ((endPos = pathString.find(';', startPos)) != std::string::npos) {
				searchPaths.push_back(pathString.substr(startPos, endPos - startPos));
				startPos = endPos + 1;
			}
			searchPaths.push_back(pathString.substr(startPos));
		}

		char dllSearchDir[MAX_PATH];
		DWORD dllSearchDirSize = GetDllDirectoryA(MAX_PATH, dllSearchDir);
		if (dllSearchDirSize > 0) {
			searchPaths.push_back(dllSearchDir);
		}

		return searchPaths;
	}

	constexpr std::string ParseArchitecture(const std::string& architecture) {
		if (architecture == "amd64" || architecture == "x64" || architecture == "x86_64") {
			return "amd64";
		}
		else if (architecture == "i386" || architecture == "x86") {
			return "i386";
		}
		return "Unknown";
	}

	std::string Loader::GetCurrentArchitecture() {
		if (sizeof(void*) == 8) {
			return "amd64";
		}
		else if (sizeof(void*) == 4) {
			return "i386";
		}
		return "Unknown"; // Thor Odinson: ...that's impossible...
	}

	std::weak_ptr<DynamicModule> Loader::LoadDynamicLinkLibrary(const std::string& moduleFile, const std::vector<std::string>& dynamicLinkFiles) {
		auto searchDirs = GetDllSearchPaths();
		bool foundDll = false;
		if (moduleFile.starts_with("./")) {
			if (fs::exists(moduleFile) && fs::is_regular_file(moduleFile)) {
				foundDll = true;
			}
		}
		else {
			for (const auto& searchDir : searchDirs) {
				fs::path searchPath(searchDir);
				fs::path moduleFilePath(moduleFile);
				fs::path moduleFile = searchPath / moduleFilePath;
				if (fs::exists(moduleFile) && fs::is_regular_file(moduleFile)) {
					foundDll = true;
					break;
				}
			}
		}
		if (!foundDll) {
			LOG_ERROR("Failed to load module '{}', file not found!", moduleFile);
			return {};
		}
		std::string moduleName = fs::path(moduleFile).filename().string();
		LOG_TRACE("Loading module '{}'...", moduleName);
		
		auto moduleHandle = LoadLibraryA(moduleFile.c_str());
		uintptr_t moduleBase = reinterpret_cast<uintptr_t>(moduleHandle);
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
			LOG_ERROR("Failed to load module '{}', windows internal error 0x{:x} '{}'...", moduleName, lastError, errorMessage);
			return {};
		}

		std::shared_ptr<DynamicModule> moduleResult = std::shared_ptr<DynamicModule>(new DynamicModule(reinterpret_cast<void*>(moduleHandle), moduleFile, {}, {}, {}, {}));
		GetDynamicLinkModuleDescriptors(*moduleResult);
		loadedDynamicModules.push_back(moduleResult);

		if (moduleResult->dynamicLinkModuleDescriptors.empty()) {
			LOG_WARN("No dynamic link modules to link against.", moduleName);
			LOG_TRACE("Module '{}' loaded successfully at 0x{:x}.", moduleName, moduleBase);
			return moduleResult;
		}

		if (dynamicLinkFiles.empty()) {
			LOG_WARN("No dynamic link files provided for '{}'.", moduleName);
			LOG_TRACE("Module '{}' loaded successfully at 0x{:x}.", moduleName, moduleBase);
			return moduleResult;
		}

		json::SchemaValidator validator(DynamicLinkModuleModel::GetSchemaDocument());
		for (const auto& dynamicLinkFile : dynamicLinkFiles) {
			validator.Reset();
			if (!fs::exists(dynamicLinkFile) || !fs::is_regular_file(dynamicLinkFile)) {
				LOG_ERROR("Failed to load dynamic link file '{}', file not found!", dynamicLinkFile);
				continue;
			}
			std::string dynamicLinkFileName = fs::path(dynamicLinkFile).filename().string();
			std::ifstream dynamicLinkFileStream(dynamicLinkFile);
			std::string dynamicLinkFileContents((std::istreambuf_iterator<char>(dynamicLinkFileStream)), std::istreambuf_iterator<char>());
			LOG_TRACE("Loading dynamic link file '{}'...", dynamicLinkFileName);
			json::Document document;
			json::ParseResult result = document.Parse(dynamicLinkFileContents.c_str());
			if (result.IsError()) {
				LOG_ERROR("Failed to parse dynamic link file '{}'...!", dynamicLinkFileName);
				LOG_ERROR("  Error: {} at {}", json::GetParseError_En(result.Code()), result.Offset());
				continue;
			}
			
			if (!document.Accept(validator)) {
				LOG_ERROR("Failed to validate dynamic link file '{}', invalid schema!", dynamicLinkFileName);
				json::StringBuffer buffer;
				validator.GetInvalidSchemaPointer().StringifyUriFragment(buffer);
				LOG_ERROR("  Error, invalid schema: {}", buffer.GetString());
				LOG_ERROR("  Error, invalid keyword: {}", validator.GetInvalidSchemaKeyword());
				buffer.Clear();
				validator.GetInvalidDocumentPointer().StringifyUriFragment(buffer);
				LOG_ERROR("  Error, invalid document: {}", buffer.GetString());
				continue;
			}

			DynamicLinkModuleModel dynamicLinkModule{
				document["target"].GetString(),
				{}
			};

			for (const auto& importJson : document["imports"].GetArray()) {
				DynamicLinkImportModel dynamicLinkImport{
					importJson.HasMember("architecture") ? importJson["architecture"].GetString() : "amd64",
					importJson["symbol"].GetString(),
					{}
				};

				for (const auto& pointer : importJson["pointers"].GetArray()) {
					if (!pointer.HasMember("version") || pointer["version"].GetString() == "ignore")
						continue;

					DynamicLinkImportPointerModel dynamicLinkImportPointer{
						pointer["version"].GetString(),
						pointer["type"].GetString(),
						pointer["value"].GetString()
					};

					dynamicLinkImport.pointers.push_back(dynamicLinkImportPointer);
				}
				dynamicLinkModule.imports.push_back(dynamicLinkImport);
			}
			moduleResult->parsedDynamicLinkModules.push_back(dynamicLinkModule);
			LOG_TRACE("Loaded dynamic link file '{}' for '{}'...", dynamicLinkFileName, moduleName);
		}

		if (DynamicallyLinkModule(*moduleResult)) {
			for (const auto& descriptor : moduleResult->dynamicLinkModuleDescriptors) {
				if (std::find(moduleResult->linkedDynamicModules.begin(), moduleResult->linkedDynamicModules.end(), descriptor) != moduleResult->linkedDynamicModules.end()) {
					LOG_TRACE("Sucessfully dynamically linked module '{}' for '{}'.", descriptor.GetImportModuleName(), moduleName);
					for (const auto& import : descriptor.importDescriptors) {
						if (std::find(moduleResult->linkedDynamicImports.begin(), moduleResult->linkedDynamicImports.end(), import) != moduleResult->linkedDynamicImports.end()) {
							LOG_TRACE("  Sucessfully imported symbol '{}' pointing to 0x{:x}.", import.importName, import.ReadAddress());
						}
						else {
							LOG_WARN("  Failed to import symbol, expect crashes trying to use this '{}'.", import.importName);
						}
					}
				}
				else {
					LOG_WARN("Failed to link module '{}' for '{}', expect crashes trying to use those: ", descriptor.GetImportModuleName(), moduleName);
					for (const auto& import : descriptor.importDescriptors) {
						LOG_WARN("  Symbol '{}'.", import.importName);
					}
				}
			}
		}
		else {
			LOG_ERROR("Failed to link dynamic modules for '{}'.", moduleName);
			LOG_ERROR("Expect pretty BAD crashes trying to use any dynamically linked function.");
		}
		return moduleResult;
	}

	void WINAPI Loader::OnLoadLibraryA(const char* name) {
		
	}

	void WINAPI Loader::OnFreeLibrary(HMODULE library) {
		char path[MAX_PATH];
		if (!GetModuleFileNameA(library, path, MAX_PATH)) {
			return;
		}
		std::string name = fs::path(path).filename().string();

		auto it = std::find_if(loadedDynamicModules.begin(), loadedDynamicModules.end(), [library](const std::shared_ptr<DynamicModule>& module) {
			return module->GetBaseAddress() == reinterpret_cast<uintptr_t>(library);
		});

		LOG_TRACE("Unloading module '{}'...", name);
		for (auto& loadedModule : loadedDynamicModules) {
			for (auto& moduleDescriptor : loadedModule->dynamicLinkModuleDescriptors) {
				if (std::find(loadedModule->linkedDynamicModules.begin(), loadedModule->linkedDynamicModules.end(), moduleDescriptor) != loadedModule->linkedDynamicModules.end()) {
					LOG_TRACE("Unlinking module '{}' from '{}'.", moduleDescriptor.GetImportModuleName(), name);
					for (auto& importDescriptor : moduleDescriptor.importDescriptors) {
						if (std::find(loadedModule->linkedDynamicImports.begin(), loadedModule->linkedDynamicImports.end(), importDescriptor) != loadedModule->linkedDynamicImports.end()) {
							LOG_TRACE("  Unlinking symbol '{}' pointing from 0x{:x} to 0x0.", importDescriptor.importName, importDescriptor.ReadAddress());
							importDescriptor.WriteAddress(0);
						}
					}
				}
			}

			for (auto it = loadedModule->linkedDynamicModules.begin(); it != loadedModule->linkedDynamicModules.end(); ) {
				if ([&name](const DynamicLinkModuleDescriptor& moduleDesc) {
					return moduleDesc.GetImportModuleName() == name;
				}(*it)) {
					it = loadedModule->linkedDynamicModules.erase(it);
				}
				else {
					++it;
				}
			}
		}
	}

	bool Loader::IsModuleValid(HMODULE moduleHandle) {
		char path[MAX_PATH];
		DWORD result = GetModuleFileNameA(moduleHandle, path, sizeof(path));
		return result != 0 && GetLastError() != ERROR_INVALID_PARAMETER;
	}
}