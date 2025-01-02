#include <windows.h>
#include <fmt/format.h>
#include <cassert>
#include "Common/ModuleUtils.hpp"
#include "Common/Log.hpp"
#include "Common/Semver.hpp"
#include "Loader/Hooks.hpp"
#include "Helpers/Helpers.hpp"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		LOG_TRACE("Loaded dynamic link module loader.");
		DynaLink::Hooks::Initialize();
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		DynaLink::Hooks::Terminate();
		LOG_TRACE("Unloaded dynamic link module loader.");
	}
    return TRUE;
}