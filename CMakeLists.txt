cmake_minimum_required(VERSION 3.20)
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