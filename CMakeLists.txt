cmake_minimum_required(VERSION 3.20)
include_directories("${CMAKE_SOURCE_DIR}/vendor/rapidjson/include")
add_definitions(-DFMT_HEADER_ONLY)
add_definitions(-UNDEBUG)
include_directories("${CMAKE_SOURCE_DIR}/vendor/fmt/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")

project(DynamicLinkLibLoader VERSION 1.0 LANGUAGES CXX)

# Specify C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to enable or disable tests
option(ENABLE_TESTS "Enable building tests" ON)

set(ARCH_BITS 0)
set(ARCH_NAME 0)
if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(ARCH_BITS 64)
    set(ARCH_NAME "x64")
elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
elseif(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    set(ARCH_BITS 32)
    set(ARCH_NAME "x86")
else()
    set(ARCH_BITS "Unknown")
    set(ARCH_NAME "Unknown")
endif()

# MSVC-specific settings to handle different build configurations
if (MSVC)
    set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)
    
    # Loop through configuration types (e.g., Debug, Release)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIGUPPER)  # Corrected this line

        # Set the output directories for the specified configurations
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} "${CMAKE_CURRENT_SOURCE_DIR}/build/${OUTPUTCONFIG}/${ARCH_NAME}")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} "${CMAKE_CURRENT_SOURCE_DIR}/build/${OUTPUTCONFIG}/${ARCH_NAME}")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} "${CMAKE_CURRENT_SOURCE_DIR}/build/${OUTPUTCONFIG}/${ARCH_NAME}")
    endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
endif()

add_subdirectory(vendor/libhat)

file(GLOB_RECURSE SOURCES_FILES "${CMAKE_CURRENT_SOURCE_DIR}/source/**.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/**.hpp")
add_library(DynamicLinkLibLoader SHARED ${SOURCES_FILES})
target_compile_definitions(DynamicLinkLibLoader PRIVATE LOADER_EXPORTS)
target_include_directories(DynamicLinkLibLoader PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_include_directories(DynamicLinkLibLoader PRIVATE "${CMAKE_SOURCE_DIR}/schemas")
target_include_directories(DynamicLinkLibLoader PRIVATE "${CMAKE_SOURCE_DIR}/vendor/minhook/include")
target_compile_definitions(DynamicLinkLibLoader PRIVATE TARGET_LOG_NAME="dynaldr${ARCH_BITS}")
target_link_libraries(DynamicLinkLibLoader PRIVATE "${CMAKE_SOURCE_DIR}/vendor/minhook/lib/libMinHook-${ARCH_NAME}-v141-mtd.lib")
set_target_properties(DynamicLinkLibLoader PROPERTIES OUTPUT_NAME "dynaldr${ARCH_BITS}")
target_compile_options(DynamicLinkLibLoader PRIVATE /Od)
target_link_libraries(DynamicLinkLibLoader PRIVATE libhat)
target_include_directories(DynamicLinkLibLoader PRIVATE "vendor/libhat/include")

if(ENABLE_TESTS)
    add_subdirectory(tests/Executable)
    add_subdirectory(tests/Library)
    add_subdirectory(tests/OtherLibrary)
    if (MSVC) 
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Executable)
    endif()
endif()