#include <windows.h>
#include <fmt/format.h>
#include <cassert>
#include "Log.hpp"
#include "Semver.hpp"
#include "Common.hpp"

extern "C" void LOADER_API ExecutablePrintHello();
extern "C" void LOADER_API ExecutablePrintWorld();
extern "C" void LOADER_API OtherLibraryPrintHello();
extern "C" void LOADER_API OtherLibraryPrintWorld();

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		LOG_TRACE("Loaded library.");
		LOG_TRACE("Function ExecutablePrintHello 0x{:x}", (uintptr_t)&ExecutablePrintHello);
		LOG_TRACE("Function ExecutablePrintWorld 0x{:x}", (uintptr_t)&ExecutablePrintWorld);
		LOG_TRACE("Function OtherLibraryPrintHello 0x{:x}", (uintptr_t)&OtherLibraryPrintHello);
		LOG_TRACE("Function OtherLibraryPrintWorld 0x{:x}", (uintptr_t)&OtherLibraryPrintWorld);
	}
	return TRUE;
}