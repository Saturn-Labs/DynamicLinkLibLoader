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

#ifdef _WIN64
#define SYMBOL_FOR_LIBLOADER_LOAD "?LoadDynamicLinkLibrary@Loader@DynaLink@@SA?AV?$weak_ptr@UDynamicModule@DynaLink@@@std@@AEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@4@AEBV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@4@@Z"
#else
#define SYMBOL_FOR_LIBLOADER_LOAD "?LoadDynamicLinkLibrary@Loader@DynaLink@@SA?AUDynamicModule@2@ABV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@ABV?$vector@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@V?$allocator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@2@@5@@Z"
#endif

int main() {
	uint64_t baseAddress = reinterpret_cast<uint64_t>(GetModuleHandle(nullptr));
	LOG_TRACE("Base address is 0x{:x}", baseAddress);
	LOG_TRACE("Size of std::string is 0x{:x}", sizeof(std::string));
	LOG_TRACE("&DataClass::PrintData is 0x{:x}", reinterpret_cast<uint64_t>(&DataClass::PrintData) - baseAddress);
	LOG_TRACE("Loading dynalink64.dll...");
	auto libLoader = LoadLibrary("dynalink64.dll");
	if (libLoader == nullptr) {
		LOG_ERROR("Failed to load dynalink64.dll.");
		return 1;
	}

	auto LoadDynamicLinkLibrary = reinterpret_cast<std::weak_ptr<DynaLink::DynamicModule>(*)(const std::string& moduleName, const std::vector<std::string>& dynamicLinkingFiles)>(GetProcAddress(libLoader, SYMBOL_FOR_LIBLOADER_LOAD));
	if (LoadDynamicLinkLibrary == nullptr) {
		LOG_ERROR("Failed to get LoadDynamicLinkLibrary function.");
		return 1;
	}
	LoadDynamicLinkLibrary("./Library.dll", {"./Executable.dynalink.json"});

	FreeLibrary(GetModuleHandleA("Library.dll"));
	std::cin.get();
	return 0;
}