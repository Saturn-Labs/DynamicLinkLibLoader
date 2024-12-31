add_library(DynamicLinkLibLoader SHARED main.cpp "Loader/Loader.cpp" "Loader/DynamicModule.cpp" "Descriptors/DynamicLinkModuleDescriptor.cpp" "Descriptors/DynamicLinkImportDescriptor.cpp")
target_compile_definitions(DynamicLinkLibLoader PRIVATE LOADER_EXPORTS)
target_include_directories(DynamicLinkLibLoader PRIVATE "${CMAKE_SOURCE_DIR}/include")