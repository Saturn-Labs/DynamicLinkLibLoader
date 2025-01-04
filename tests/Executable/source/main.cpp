#include <filesystem>
#include <iostream>
#include <windows.h>
#include <type_traits>
#include "Log.hpp"
namespace fs = std::filesystem;

#ifdef _WIN64
#define DYNALINK_DLL "dynaldr64.dll"
#else
#define DYNALINK_DLL "dynaldr32.dll"
#endif

void PrintHello() {
	LOG_TRACE("Hello from Executable.exe!");
}

void PrintWorld() {
	LOG_TRACE("World from Executable.exe!");
}

int main() {
	&PrintHello;
	&PrintWorld;
	uint64_t baseAddress = reinterpret_cast<uint64_t>(GetModuleHandle(nullptr));
	LOG_TRACE("Base address is 0x{:x}", baseAddress);
	LOG_TRACE("Loading {}...", DYNALINK_DLL);
	auto libLoader = LoadLibrary(DYNALINK_DLL);
	if (libLoader == nullptr) {
		LOG_ERROR("Failed to load {}.", DYNALINK_DLL);
		return 1;
	}

	auto LoadDynamicLinkLibrary = reinterpret_cast<HMODULE(*)(const std::string& moduleName, const std::vector<std::string>& dynamicLinkingFiles, uint32_t* result)>(GetProcAddress(libLoader, "LoadDynamicLinkLibrary"));
	if (LoadDynamicLinkLibrary == nullptr) {
		LOG_ERROR("Failed to get LoadDynamicLinkLibrary function.");
		return 1;
	}
	LoadDynamicLinkLibrary("./Library.dll", {"../../../tests/Library/links/Executable.link.json"}, nullptr);
	//FreeLibrary(GetModuleHandleA("Library.dll"));
	std::cin.get();
	return 0;
}