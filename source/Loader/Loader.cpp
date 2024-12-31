#include "Common/Loader/Loader.hpp"
#include "Common/Loader/DynamicModule.hpp"
#include "Common/Log.hpp"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <fstream>
namespace json = rapidjson;

namespace DynaLink {
	DynamicModule Loader::LoadDynamicLinkLibrary(const std::string& moduleFile, const std::vector<std::string>& dynamicLinkFiles) {
		if (!fs::exists(moduleFile) || !fs::is_regular_file(moduleFile)) {
			LOG_ERROR("[DynamicLinkLibLoader] Failed to load module '{}', file not found!", moduleFile);
			return { nullptr, "", {} };
		}
		std::string moduleName = fs::path(moduleFile).filename().string();
		LOG_TRACE("[DynamicLinkLibLoader] Loading module '{}'...", moduleName);
		
		auto moduleHandle = LoadLibraryA(moduleFile.c_str());
		uint64_t moduleBase = reinterpret_cast<uint64_t>(moduleHandle);
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
			return { nullptr, "", {} };
		}

		if (dynamicLinkFiles.empty()) {
			LOG_WARN("[DynamicLinkLibLoader] No dynamic link files provided for '{}'.", moduleName);
			LOG_TRACE("[DynamicLinkLibLoader] Module '{}' loaded successfully at 0x{:x}.", moduleName, moduleBase);
			return { moduleHandle, moduleFile, {} };
		}

		std::vector<DynamicLinkModuleModel> dynamicLinkModules;
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
			dynamicLinkModules.push_back(dynamicLinkModule);
		}
	}
}