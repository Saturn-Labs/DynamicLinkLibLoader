#pragma once
#include <string>
#include "DynamicSymbolModel.hpp"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/schema.h>
namespace json = rapidjson;

namespace DynaLink {
    struct DynamicImportModel {
        std::string descriptorPath;
        std::string target;
        std::vector<DynamicSymbolModel> symbols;

        static std::string GetSchema();
        static json::SchemaDocument& GetSchemaDocument() {
            static bool initialized = false;
            static json::Document document;
            if (!initialized) {
                document.Parse(GetSchema().c_str());
                initialized = true;
            }

            static json::SchemaDocument schemaDocument(document);
            return schemaDocument;
        }
    };
}