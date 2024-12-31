#include <windows.h>
#include <fmt/format.h>
#include <Common/Log.hpp>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		LOG_TRACE("[DynamicLinkLibLoader] Loaded custom module loader.");
		LOG_TRACE("[DynamicLinkLibLoader] Size of std::string is 0x{:x}", sizeof(std::string));
	}
    return TRUE;
}