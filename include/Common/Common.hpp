#pragma once
#ifdef LOADER_EXPORTS
#define LOADER_API __declspec(dllexport)
#else
#define LOADER_API __declspec(dllimport)
#endif
#include <vector>
#include <string>
#include <filesystem>
#include <rapidjson/rapidjson.h>
#include <optional>
#define STRINGIFY(...) #__VA_ARGS__
#define STR(...) STRINGIFY(__VA_ARGS__)

namespace fs = std::filesystem;
namespace json = rapidjson;