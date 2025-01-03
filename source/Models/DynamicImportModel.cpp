#include <Models/DynamicImportModel.hpp>
#include <Common/Common.hpp>
#include <Common/Log.hpp>
#include "Common/ArchUtils.hpp"

namespace DynaLink {
    std::string DynamicImportModel::GetSchema() {
        return R"(
        {
            "$id": "dynamic-linker-symbol-file",
            "$schema": "http://json-schema.org/draft-04/schema#",
            "type": "object",
            "properties": {
                "target": {
                    "type": "string"
                },
                "default_version": {
                    "type": "string",
                    "value": { "pattern": "^(ignore|(\\d+|\\*)(\\.(\\d+|\\*)){0,3})$" }
                },
                "symbols": {
                    "type": "array",
                    "items": {
                        "type": "object",
                        "properties": {
                            "version": {
                                "type": "string",
                                "value": { "pattern": "^(ignore|(\\d+|\\*)(\\.(\\d+|\\*)){0,3})$" }
                            },
                            "architecture": {
                                "type": "string",
                                "enum": ["amd64", "x64", "x86_64", "x86", "i386"]
                            },
                            "symbol": {
                                "type": "string"
                            },
                            "pointer": {
                                "properties": {
                                    "type": { "type": "string", "enum": ["offset", "pattern"] },
                                    "value": { "type": "string" }
                                },
                       	        "required": ["value", "type"],
                                "oneOf": [
                                    {
                                        "properties": {
                                            "type": { "enum": ["offset"] },
                                            "value": { "pattern": "^0x[0-9a-fA-F]{1,16}$" }
                                        }
                                    },
                                    {
                                        "properties": {
                                            "type": { "enum": ["pattern"] },
                                            "value": { "pattern": "^([0-9A-Fa-f]{2}|\\?)(( [0-9A-Fa-f]{2}| \\?))*$" }
                                        }
                                    }
                                ]
                            }
                        },
                        "required": ["symbol", "pointer"],
                        "default": {
                            "architecture": "amd64"
                        }
                    }
                }
            },
            "required": ["target", "symbols"]
        })";
    }

    json::SchemaDocument& DynamicImportModel::GetSchemaDocument() {
        static bool initialized = false;
        static json::Document document;
        if (!initialized) {
            document.Parse(GetSchema().c_str());
            initialized = true;
        }

        static json::SchemaDocument schemaDocument(document);
        return schemaDocument;
    }

    std::optional<DynamicImportModel> DynamicImportModel::ParseFromContents(const std::string& contents)
    {
        static json::SchemaValidator validator(GetSchemaDocument());
        validator.Reset();

        json::Document document;
        json::ParseResult result = document.Parse(contents.c_str());
        if (result.IsError()) {
            LOG_ERROR("Failed to parse dynamic link file...!");
            LOG_ERROR("  Error: {} at {}", json::GetParseError_En(result.Code()), result.Offset());
            return std::nullopt;
        }

        if (!document.Accept(validator)) {
            LOG_ERROR("Failed to validate dynamic link file, invalid schema!");
            json::StringBuffer buffer;
            validator.GetInvalidSchemaPointer().StringifyUriFragment(buffer);
            LOG_ERROR("  Error, invalid schema: {}", buffer.GetString());
            LOG_ERROR("  Error, invalid keyword: {}", validator.GetInvalidSchemaKeyword());
            buffer.Clear();
            validator.GetInvalidDocumentPointer().StringifyUriFragment(buffer);
            LOG_ERROR("  Error, invalid document: {}", buffer.GetString());
            return std::nullopt;
        }

        DynamicImportModel dynamicImportModel{
            document["target"].GetString(),
			document.HasMember("default_version") ? document["default_version"].GetString() : "*",
            {}
        };

        for (const auto& symbolJson : document["symbols"].GetArray()) {
            DynamicSymbolModel dynamicSymbolModel{
                symbolJson.HasMember("version") ? symbolJson["version"].GetString() : dynamicImportModel.defaultVersion,
                symbolJson.HasMember("architecture") ? (ArchUtils::IsArchitectureValid(symbolJson["architecture"].GetString()) ? ArchUtils::NormalizeArchitecture(symbolJson["architecture"].GetString()) : "amd64") : "amd64",
                symbolJson["symbol"].GetString(),
                {}
            };

            auto pointerJsonObj = symbolJson["pointer"].GetObject();
            DynamicSymbolPointerModel dynamicSymbolPointerModel{
                pointerJsonObj["type"].GetString(),
                pointerJsonObj["value"].GetString()
            };
			dynamicSymbolModel.pointer = dynamicSymbolPointerModel;
            dynamicImportModel.symbols.insert({ dynamicSymbolModel.symbol, dynamicSymbolModel });
        }
		return dynamicImportModel;
    }

    std::optional<DynamicImportModel> DynamicImportModel::ParseFromFile(const std::string& path)
    {
        if (!fs::exists(path) || !fs::is_regular_file(path)) {
            LOG_ERROR("Failed to load dynamic link file '{}', file not found!", path);
            return std::nullopt;
        }

        std::string dynamicLinkFileName = fs::path(path).filename().string();
        std::ifstream dynamicLinkFileStream(path);
        std::string dynamicLinkFileContents((std::istreambuf_iterator<char>(dynamicLinkFileStream)), std::istreambuf_iterator<char>());
        LOG_TRACE("Loading dynamic link file '{}'...", dynamicLinkFileName);
        return ParseFromContents(dynamicLinkFileContents);
    }
}