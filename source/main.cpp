#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Initialization when the DLL is loaded into a process
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        // Thread-specific initialization and cleanup (not needed in this example)
        break;
    case DLL_PROCESS_DETACH:
        // Cleanup when the DLL is unloaded
        break;
    }
    return TRUE;  // Return TRUE to indicate successful initialization
}