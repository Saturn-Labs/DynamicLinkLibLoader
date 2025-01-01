#include <windows.h>
#include <fmt/format.h>
#include <cassert>
#include "Common/Log.hpp"
#include "Common/Semver.hpp"
#include "Loader/Hooks.hpp"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		LOG_TRACE("Loaded custom module loader.");
		LOG_TRACE("Size of std::string is 0x{:x}", sizeof(std::string));
		DynaLink::Hooks::Initialize();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		DynaLink::Hooks::Terminate();
		LOG_TRACE("Unloaded custom module loader.");
	}
    return TRUE;
}