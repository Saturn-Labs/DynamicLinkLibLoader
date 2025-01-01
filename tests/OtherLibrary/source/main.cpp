#include <windows.h>
#include <fmt/format.h>
#include <cassert>
#include "Log.hpp"
#include "Semver.hpp"

extern "C" __declspec(dllexport) void PrintFromOtherLibrary() {
	LOG_TRACE("Printing from other library.");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		LOG_TRACE("Loaded other library.");
		LOG_TRACE("&PrintFromOtherLibrary is 0x{:x} <- (0x{:x} + 0x{:x})", reinterpret_cast<uintptr_t>(&PrintFromOtherLibrary), reinterpret_cast<uintptr_t>(hModule), reinterpret_cast<uintptr_t>(&PrintFromOtherLibrary) - reinterpret_cast<uintptr_t>(hModule));
	}
	return TRUE;
}