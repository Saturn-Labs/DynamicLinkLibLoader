#include "Common/ModuleUtils.hpp"
#include "Common/Log.hpp"
#include "Helpers/Helpers.hpp"
#include <psapi.h>
#include <winternl.h>
#include <ntstatus.h>
#pragma comment(lib, "version.lib")

namespace DynaLink {
	bool ModuleUtils::IsModuleValid(HMODULE moduleHandle) {
		char moduleFileName[MAX_PATH];
		if (!moduleHandle || (GetModuleFileNameA(moduleHandle, moduleFileName, MAX_PATH) == 0 && (GetLastError() == ERROR_MOD_NOT_FOUND || GetLastError() == ERROR_INVALID_PARAMETER))) {
			return false;
		}
		return true;
	}

	std::vector<HMODULE> ModuleUtils::GetLoadedModules() {
		std::vector<HMODULE> loadedModules{};
		DWORD processId = GetCurrentProcessId();
		HMODULE hModules[1024];
		DWORD cbNeeded;
		if (EnumProcessModules(GetCurrentProcess(), hModules, sizeof(hModules), &cbNeeded)) {
			for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
				loadedModules.push_back(hModules[i]);
			}
		}
		return loadedModules;
	}

	IMAGE_SECTION_HEADER* ModuleUtils::GetDynamicLinkSection(HMODULE moduleHandle)
	{
		if (!IsModuleValid(moduleHandle)) {
			return nullptr;
		}

		IMAGE_NT_HEADERS* ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<uintptr_t>(moduleHandle) + reinterpret_cast<IMAGE_DOS_HEADER*>(reinterpret_cast<uintptr_t>(moduleHandle))->e_lfanew);
		uint16_t sectionCount = ntHeaders->FileHeader.NumberOfSections;
		IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeaders);
		for (uint16_t i = 0; i < sectionCount; i++) {
			if (strcmp(reinterpret_cast<const char*>(section->Name), ".dlink") == 0) {
				return section;
			}
			++section;
		}
		return nullptr;
	}

	bool ModuleUtils::IsDynaLinkModuleFile(std::ifstream& stream)
	{
		IMAGE_DOS_HEADER dosHeader;
		stream.seekg(0, std::ios::beg);
		stream.read(reinterpret_cast<char*>(&dosHeader), sizeof(IMAGE_DOS_HEADER));
		if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
			return false;
		}

		IMAGE_NT_HEADERS ntHeaders;
		stream.seekg(dosHeader.e_lfanew, std::ios::beg);
		stream.read(reinterpret_cast<char*>(&ntHeaders), sizeof(IMAGE_NT_HEADERS));
		if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
			return false;
		}

		uint16_t sectionCount = ntHeaders.FileHeader.NumberOfSections;
		IMAGE_SECTION_HEADER sectionHeader;
		stream.seekg(dosHeader.e_lfanew + sizeof(IMAGE_NT_HEADERS), std::ios::beg);

		for (uint16_t i = 0; i < sectionCount; i++) {
			stream.read(reinterpret_cast<char*>(&sectionHeader), sizeof(IMAGE_SECTION_HEADER));
			if (std::strcmp(reinterpret_cast<const char*>(sectionHeader.Name), ".dlink") == 0) {
				return true;
			}
		}

		return false;
	}

	bool ModuleUtils::IsDynaLinkModuleFile(const std::string& moduleFile)
	{
		std::string path = moduleFile;
		if (!FindModuleOnStandardPaths(moduleFile, &path)) {
			return false;
		}

		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file) {
			return false;
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		file.seekg(0, std::ios::beg);
		return IsDynaLinkModuleFile(file);
	}

	std::vector<std::string> ModuleUtils::GetModuleStandardSearchPaths()
	{
		std::vector<std::string> searchPaths;
		char currentProcessDir[MAX_PATH];
		if (GetModuleFileNameA(NULL, currentProcessDir, MAX_PATH) > 0) {
			std::string processDir(currentProcessDir);
			size_t pos = processDir.find_last_of("\\/");
			if (pos != std::string::npos) {
				searchPaths.push_back(processDir.substr(0, pos));
			}
		}

		char currentDir[MAX_PATH];
		if (GetCurrentDirectoryA(MAX_PATH, currentDir) > 0) {
			searchPaths.push_back(currentDir);
		}

		char systemDir[MAX_PATH];
		if (GetSystemDirectoryA(systemDir, MAX_PATH) > 0) {
			searchPaths.push_back(systemDir);
		}

		char windowsDir[MAX_PATH];
		if (GetWindowsDirectoryA(windowsDir, MAX_PATH) > 0) {
			searchPaths.push_back(windowsDir);
		}

		const char* pathEnv = std::getenv("PATH");
		if (pathEnv != nullptr) {
			std::string pathString(pathEnv);
			size_t startPos = 0, endPos;
			while ((endPos = pathString.find(';', startPos)) != std::string::npos) {
				searchPaths.push_back(pathString.substr(startPos, endPos - startPos));
				startPos = endPos + 1;
			}
			searchPaths.push_back(pathString.substr(startPos));
		}

		char dllSearchDir[MAX_PATH];
		DWORD dllSearchDirSize = GetDllDirectoryA(MAX_PATH, dllSearchDir);
		if (dllSearchDirSize > 0) {
			searchPaths.push_back(dllSearchDir);
		}

		return searchPaths;
	}

	bool ModuleUtils::FindModuleOnStandardPaths(const std::string& moduleFile, std::string* newPath)
	{
		auto searchDirs = GetModuleStandardSearchPaths();
		std::string fixedModulePath = moduleFile;
		bool foundDll = false;
		if (moduleFile.starts_with("./")) {
			if (fs::exists(moduleFile) && fs::is_regular_file(moduleFile)) {
				foundDll = true;
			}
		}
		else {
			for (const auto& searchDir : searchDirs) {
				fs::path searchPath(searchDir);
				fs::path moduleFilePath(moduleFile);
				fs::path moduleFile = searchPath / moduleFilePath;
				if (fs::exists(moduleFile) && fs::is_regular_file(moduleFile)) {
					fixedModulePath = moduleFile.string();
					foundDll = true;
					break;
				}
			}
		}
		if (newPath != nullptr) {
			*newPath = fixedModulePath;
		}
		return foundDll;
	}

	Semver ModuleUtils::GetModuleVersion(HMODULE moduleHandle) {
		static Semver any("*");
		char path[MAX_PATH];
		if (!GetModuleFileNameA(moduleHandle, path, MAX_PATH)) {
			return any;
		}

		DWORD versionHandle = 0;
		DWORD size = GetFileVersionInfoSizeA(path, &versionHandle);
		if (size == 0) {
			return any;
		}

		char* versionData = new char[size];
		if (!GetFileVersionInfoA(path, versionHandle, size, versionData)) {
			delete[] versionData;
			return any;
		}

		VS_FIXEDFILEINFO* fileInfo;
		UINT fileInfoSize;

		if (!VerQueryValueA(versionData, "\\", (LPVOID*)&fileInfo, &fileInfoSize)) {
			delete[] versionData;
			return any;
		}

		DWORD major = HIWORD(fileInfo->dwFileVersionMS);
		DWORD minor = LOWORD(fileInfo->dwFileVersionMS);
		DWORD build = HIWORD(fileInfo->dwFileVersionLS);
		DWORD revision = LOWORD(fileInfo->dwFileVersionLS);
		return Semver(fmt::format("{}.{}.{}.{}", major, minor, build, revision));
	}
}