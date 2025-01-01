#pragma once
#include <string>
#include "DynamicLinkImportModel.hpp"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/schema.h>
namespace json = rapidjson;

namespace DynaLink {
    struct DynamicLinkModuleModel {
        std::string descriptorPath;
        std::string target;
        std::vector<DynamicLinkImportModel> imports;

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