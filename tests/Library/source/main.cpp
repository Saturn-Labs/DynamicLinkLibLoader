#include <windows.h>
#include <fmt/format.h>
#include <cassert>
#include "Log.hpp"
#include "Semver.hpp"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		LOG_TRACE("Loaded library.");
	}
	return TRUE;
}