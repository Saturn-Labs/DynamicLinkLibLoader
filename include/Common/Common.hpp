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
#include <unordered_map>
#include <optional>
#include <fstream>
#define STRINGIFY(...) #__VA_ARGS__
#define STR(...) STRINGIFY(__VA_ARGS__)

namespace fs = std::filesystem;
namespace json = rapidjson;

namespace DynaLink {
	using i8 = int8_t;
	using i16 = int16_t;
	using i32 = int32_t;
	using i64 = int64_t;
	using u8 = uint8_t;
	using u16 = uint16_t;
	using u32 = uint32_t;
	using u64 = uint64_t;
	using uptr = uintptr_t;
	using iptr = intptr_t;
}