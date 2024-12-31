#include "Common/Loader/Loader.hpp"
#include "Common/Loader/DynamicModule.hpp"
#include "Common/Log.hpp"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <fstream>
namespace json = rapidjson;

namespace DynaLink {
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
			auto descriptor = DynamicLinkModuleDescriptor::Create(dynamicModule, *dynamicImport);
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

			HMODULE importModule = GetModuleHandleA(descriptor.GetImportModuleName());
			if (importModule == nullptr) {
				LOG_ERROR("[DynamicLinkLibLoader] Failed to link module '{}' functions, required by '{}'... Module not found!", descriptor.GetImportModuleName(), dynamicModule.moduleFile);
				continue;
			}

			Semver importModuleVersion = GetModuleVersion(importModule);
			auto moduleModelIterator = std::find_if(dynamicModule.parsedDynamicLinkModules.begin(), dynamicModule.parsedDynamicLinkModules.end(), [&descriptor](const DynamicLinkModuleModel& model) {
				return model.target == descriptor.GetImportModuleName();
			});

			if (moduleModelIterator == dynamicModule.parsedDynamicLinkModules.end()) {
				LOG_ERROR("[DynamicLinkLibLoader] Failed to link module '{}' functions, required by '{}'... Dynamic Linking file not found!", descriptor.GetImportModuleName(), dynamicModule.moduleFile);
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
							LOG_WARN("[DynamicLinkLibLoader] Failed to link module '{}' function '{}', required by '{}'... Pattern linking not supported!", descriptor.GetImportModuleName(), import.importName, dynamicModule.moduleFile);
						}
						break;
					}
				}
			}
			dynamicModule.linkedDynamicModules.push_back(descriptor);
		}
		return (dynamicModule.dynamicLinkModuleDescriptors.size() > 1 && dynamicModule.linkedDynamicModules.size() > 1);
	}

	DynamicModule Loader::LoadDynamicLinkLibrary(const std::string& moduleFile, const std::vector<std::string>& dynamicLinkFiles) {
		DynamicModule moduleResult{ nullptr, "", {} };
		if (!fs::exists(moduleFile) || !fs::is_regular_file(moduleFile)) {
			LOG_ERROR("[DynamicLinkLibLoader] Failed to load module '{}', file not found!", moduleFile);
			return moduleResult;
		}
		std::string moduleName = fs::path(moduleFile).filename().string();
		LOG_TRACE("[DynamicLinkLibLoader] Loading module '{}'...", moduleName);
		
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
			LOG_ERROR("[DynamicLinkLibLoader] Failed to load module '{}', windows internal error 0x{:x} '{}'...", moduleName, lastError, errorMessage);
			return moduleResult;
		}

		moduleResult.moduleHandle = moduleHandle;
		moduleResult.moduleFile = moduleFile;
		GetDynamicLinkModuleDescriptors(moduleResult);

		if (dynamicLinkFiles.empty()) {
			LOG_WARN("[DynamicLinkLibLoader] No dynamic link files provided for '{}'.", moduleName);
			LOG_TRACE("[DynamicLinkLibLoader] Module '{}' loaded successfully at 0x{:x}.", moduleName, moduleBase);
			return moduleResult;
		}

		json::SchemaValidator validator(DynamicLinkModuleModel::GetSchemaDocument());
		for (const auto& dynamicLinkFile : dynamicLinkFiles) {
			validator.Reset();
			if (!fs::exists(dynamicLinkFile) || !fs::is_regular_file(dynamicLinkFile)) {
				LOG_ERROR("[DynamicLinkLibLoader] Failed to load dynamic link file '{}', file not found!", dynamicLinkFile);
				continue;
			}
			std::string dynamicLinkFileName = fs::path(dynamicLinkFile).filename().string();
			std::ifstream dynamicLinkFileStream(dynamicLinkFile);
			std::string dynamicLinkFileContents((std::istreambuf_iterator<char>(dynamicLinkFileStream)), std::istreambuf_iterator<char>());
			LOG_TRACE("[DynamicLinkLibLoader] Loading dynamic link file '{}'...", dynamicLinkFileName);
			json::Document document;
			json::ParseResult result = document.Parse(dynamicLinkFileContents.c_str());
			if (result.IsError()) {
				LOG_ERROR("[DynamicLinkLibLoader] Failed to parse dynamic link file '{}'...!", dynamicLinkFileName);
				LOG_ERROR("[DynamicLinkLibLoader]	Error: {} at {}", json::GetParseError_En(result.Code()), result.Offset());
				continue;
			}
			
			if (!document.Accept(validator)) {
				LOG_ERROR("[DynamicLinkLibLoader] Failed to validate dynamic link file '{}', invalid schema!", dynamicLinkFileName);
				json::StringBuffer buffer;
				validator.GetInvalidSchemaPointer().StringifyUriFragment(buffer);
				LOG_ERROR("[DynamicLinkLibLoader]	Error, invalid schema: {}", buffer.GetString());
				LOG_ERROR("[DynamicLinkLibLoader]	Error, invalid keyword: {}", validator.GetInvalidSchemaKeyword());
				buffer.Clear();
				validator.GetInvalidDocumentPointer().StringifyUriFragment(buffer);
				LOG_ERROR("[DynamicLinkLibLoader]	Error, invalid document: {}", buffer.GetString());
				continue;
			}

			DynamicLinkModuleModel dynamicLinkModule{
				document["target"].GetString(),
				{}
			};

			for (const auto& import : document["imports"].GetArray()) {
				DynamicLinkImportModel dynamicLinkImport{
					import["symbol"].GetString(),
					{}
				};

				for (const auto& pointer : import["pointers"].GetArray()) {
					DynamicLinkImportPointerModel dynamicLinkImportPointer{
						pointer["version"].GetString(),
						pointer["type"].GetString(),
						pointer["value"].GetString()
					};

					dynamicLinkImport.pointers.push_back(dynamicLinkImportPointer);
				}
				dynamicLinkModule.imports.push_back(dynamicLinkImport);
			}
			moduleResult.parsedDynamicLinkModules.push_back(dynamicLinkModule);
			LOG_TRACE("[DynamicLinkLibLoader] Loaded dynamic link file '{}' for '{}'...", dynamicLinkFileName, moduleName);
		}

		if (DynamicallyLinkModule(moduleResult)) {
			for (const auto& descriptor : moduleResult.dynamicLinkModuleDescriptors) {
				if (std::find(moduleResult.linkedDynamicModules.begin(), moduleResult.linkedDynamicModules.end(), descriptor) != moduleResult.linkedDynamicModules.end()) {
					LOG_TRACE("[DynamicLinkLibLoader] Sucessfully dynamically linked module '{}' for '{}'.", descriptor.GetImportModuleName(), moduleName);
					for (const auto& import : descriptor.importDescriptors) {
						if (std::find(moduleResult.linkedDynamicImports.begin(), moduleResult.linkedDynamicImports.end(), import) != moduleResult.linkedDynamicImports.end()) {
							LOG_TRACE("[DynamicLinkLibLoader]   Sucessfully imported symbol '{}' pointing to 0x{:x}.", import.importName, import.ReadAddress());
						}
						else {
							LOG_WARN("[DynamicLinkLibLoader]   Failed to import symbol, expect crashes trying to use this '{}'.", import.importName);
						}
					}
				}
				else {
					LOG_WARN("[DynamicLinkLibLoader] Failed to link module '{}' for '{}', expect crashes trying to use those: ", descriptor.GetImportModuleName(), moduleName);
					for (const auto& import : descriptor.importDescriptors) {
						LOG_WARN("[DynamicLinkLibLoader]   Symbol '{}'.", import.importName);
					}
				}
			}
		}
		else {
			LOG_ERROR("[DynamicLinkLibLoader] Failed to link dynamic modules for '{}'.", moduleName);
			LOG_ERROR("[DynamicLinkLibLoader] Expect pretty BAD crashes trying to use any dynamically linked function.");
		}
		return moduleResult;
	}
}