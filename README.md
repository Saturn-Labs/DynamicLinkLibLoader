# DynamicLinkLibLoader
Custom Library Loader that supports linking imports at runtime with a json file, meant to be used with https://github.com/Saturn-Labs/DynamicLinker

Dynamic Linker Symbol File Schema
```json
{
    "$id": "dynamic-linker-symbol-file",
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
	                    "architecture": {
    	                    "type": "string",
     	                    "enum": ["amd64", "x64", "x86_64", "x86", "i386"]
                    },
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
                        },
                        "minItems": 1
                    }
                },
                "required": ["symbol", "pointers"],
                "default": {
                    "architecture": "amd64"
                }
            }
        }
    },
    "required": ["target", "imports"]
}
