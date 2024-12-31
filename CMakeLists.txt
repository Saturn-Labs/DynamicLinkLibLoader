cmake_minimum_required(VERSION 3.20)

include_directories("${CMAKE_SOURCE_DIR}/vendor/rapidjson/include")
add_definitions(-DFMT_HEADER_ONLY)
add_definitions(-UNDEBUG)
include_directories("${CMAKE_SOURCE_DIR}/vendor/fmt/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")

project(DynamicLinkLibLoader VERSION 1.0 LANGUAGES CXX)

# MSVC-specific settings to handle different build configurations
if (MSVC)
    set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)
    
    # Loop through configuration types (e.g., Debug, Release)
    foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIGUPPER)  # Corrected this line

        # Set the output directories for the specified configurations
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} "${CMAKE_CURRENT_SOURCE_DIR}/build/${OUTPUTCONFIG}")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} "${CMAKE_CURRENT_SOURCE_DIR}/build/${OUTPUTCONFIG}")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} "${CMAKE_CURRENT_SOURCE_DIR}/build/${OUTPUTCONFIG}")
    endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)
endif()

# Specify C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option to enable or disable tests
option(ENABLE_TESTS "Enable building tests" ON)

# Add subdirectory for source code
add_subdirectory(source)

# Optionally, add a tests subdirectory if ENABLE_TESTS is true
if (ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests/Executable)
    add_subdirectory(tests/Library)
    if (MSVC) 
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Executable)
    endif()
endif()