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

        static constexpr std::string GetSchema() {
            return R"(
            {
                "$schema": "http://json-schema.org/draft-04/schema#",
                "type": "object",
                "properties": {
                    "target": {
                        "type": "string"
                    },
                    "imports": {
                        "type": "array",
                        "items": {
                            "type": "object",
                            "properties": {
                                "symbol": {
                                    "type": "string"
                                },
                                "pointers": {
                                    "type": "array",
                                    "items": {
                                        "type": "object",
                                        "properties": {
                                            "version": {
                                                "type": "string"
                                            },
                                            "type": {
                                                "type": "string",
                                                "enum": ["offset", "pattern"]
                                            },
                                            "value": {
                                                "type": "string"
                                            }
                                        },
                                        "required": ["value", "type"],
                                        "oneOf": [
                                            {
                                                "properties": {
                                                    "type": { "enum": ["offset"] },
                                                    "value": { "pattern": "^0x[0-9a-fA-F]+$" }
                                                }
                                            },
                                            {
                                                "properties": {
                                                    "type": { "enum": ["pattern"] },
                                                    "value": { "pattern": "^([0-9A-Fa-f]{2}|\\?)(( [0-9A-Fa-f]{2}| \\?))*$" }
                                                }
                                            }
                                        ]
                                    },
                                    "minItems": 1
                                }
                            },
                            "required": ["symbol", "pointers"]
                        }
                    }
                },
                "required": ["target", "imports"]
            })";
        }

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