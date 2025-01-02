#pragma once
#include <windows.h>
#include <Loader/Loader.hpp>
#include <Common/Log.hpp>
#include <MinHook.h>
#include <winternl.h>
#include <ntstatus.h>

namespace DynaLink::Hooks {
	inline static HMODULE(WINAPI*__LoadLibraryA)(const char*) = nullptr;
	static HMODULE WINAPI _LoadLibraryA(const char* name) {
		Loader::OnLoadLibraryA(name);
		return __LoadLibraryA(name);
	}

	inline static BOOL(WINAPI*__FreeLibrary)(HMODULE) = nullptr;
	static BOOL WINAPI _FreeLibrary(HMODULE library) {
		Loader::OnFreeLibrary(library);
		return __FreeLibrary(library);
	}

	static void Initialize() {
		if (MH_Initialize() != MH_OK)
		{
			LOG_ERROR("Failed to initialize MinHook.");
			return;
		}

		if (MH_CreateHook(&LoadLibraryA, &_LoadLibraryA, reinterpret_cast<LPVOID*>(&__LoadLibraryA)) != MH_OK)
		{
			LOG_ERROR("Failed to create hook for LoadLibraryA.");
			return;
		}

		if (MH_CreateHook(&FreeLibrary, &_FreeLibrary, reinterpret_cast<LPVOID*>(&__FreeLibrary)) != MH_OK)
		{
			LOG_ERROR("Failed to create hook for FreeLibrary.");
			return;
		}

		if (MH_EnableHook(&LoadLibraryA) != MH_OK)
		{
			LOG_ERROR("Failed to enable hook for LoadLibraryA.");
			return;
		}

		if (MH_EnableHook(&FreeLibrary) != MH_OK)
		{
			LOG_ERROR("Failed to enable hook for FreeLibrary.");
			return;
		}
	}

	static void Terminate() {
		if (MH_DisableHook(&LoadLibraryA) != MH_OK)
		{
			LOG_ERROR("Failed to disable hook for LoadLibraryA.");
			return;
		}

		if (MH_DisableHook(&FreeLibrary) != MH_OK)
		{
			LOG_ERROR("Failed to disable hook for FreeLibrary.");
			return;
		}

		if (MH_Uninitialize() != MH_OK)
		{
			LOG_ERROR("Failed to uninitialize MinHook.");
			return;
		}
	}
}