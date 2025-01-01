add_library(OtherLibrary SHARED "source/main.cpp")
target_compile_definitions(OtherLibrary PRIVATE TARGET_LOG_NAME="OtherLibrary")
target_include_directories(OtherLibrary PRIVATE "${CMAKE_SOURCE_DIR}/include/Common")