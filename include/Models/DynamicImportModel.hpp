#pragma once
#include <string>
#include "DynamicSymbolModel.hpp"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <optional>
#include <unordered_map>
namespace json = rapidjson;

namespace DynaLink {
    struct DynamicImportModel {
        std::string descriptorPath;
        std::string target;
        std::string defaultVersion;
        std::unordered_map<std::string, DynamicSymbolModel> symbols;

        static std::string GetSchema();
        static json::SchemaDocument& GetSchemaDocument();
        static std::optional<DynamicImportModel> ParseFromContents(const std::string& contents);
        static std::optional<DynamicImportModel> ParseFromFile(const std::string& path);
    };
}