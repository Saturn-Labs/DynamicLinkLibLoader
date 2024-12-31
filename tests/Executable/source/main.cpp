#include <filesystem>
#include <iostream>
#include <windows.h>
#include <type_traits>
#include "Log.hpp"
namespace fs = std::filesystem;

class DataClass {
public:
	int sMyData;
	DataClass(int data);
	~DataClass();
	static void PrintData(const DataClass& dataClass);
};

DataClass::DataClass(int data) : sMyData(data) {
	LOG_TRACE("[DataClass] Constructor called with data: {}", data);
}

DataClass::~DataClass() {
	LOG_TRACE("[DataClass] Destructor called.");
}

void DataClass::PrintData(const DataClass& dataClass) {
	LOG_TRACE("[DataClass] Data is: {}", dataClass.sMyData);
}

namespace DynaLink {
	struct DynamicLinkModuleModel;
	struct DynamicLinkModuleDescriptor;
	struct DynamicLinkImportDescriptor;
	struct DynamicModule {
		char filler[136];
	};
}

int main() {
	uint64_t baseAddress = reinterpret_cast<uint64_t>(GetModuleHandle(nullptr));
	LOG_TRACE("[Test Executable] Base address is 0x{:x}", baseAddress);
	LOG_TRACE("[Test Executable] Size of std::string is 0x{:x}", sizeof(std::string));
	LOG_TRACE("[Test Executable] &DataClass::PrintData is 0x{:x}", reinterpret_cast<uint64_t>(&DataClass::PrintData) - baseAddress);
	LOG_TRACE("[Test Executable] Loading DynamicLinkLibLoader.dll...");
	auto libLoader = LoadLibrary("DynamicLinkLibLoader.dll");
	if (libLoader == nullptr) {
		LOG_ERROR("[Test Executable] Failed to load DynamicLinkLibLoader.dll.");
		return 1;
	}

	auto LoadDynamicLinkLibrary = reinterpret_cast<DynaLink::DynamicModule(*)(const std::string& moduleName, const std::vector<std::string>& dynamicLinkingFiles)>(GetProcAddress(libLoader, "?LoadDynamicLinkLibrary@Loader@DynaLink@@SA?AUDynamicModule@2@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@AEBV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@5@@Z"));
	if (LoadDynamicLinkLibrary == nullptr) {
		LOG_ERROR("[Test Executable] Failed to get LoadDynamicLinkLibrary function.");
		return 1;
	}
	LoadDynamicLinkLibrary("./Library.dll", {"./Executable.dynalink.json"});
	std::cin.get();
	return 0;
}