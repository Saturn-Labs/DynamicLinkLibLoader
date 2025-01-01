add_library(Library SHARED "source/main.cpp")
target_compile_definitions(Library PRIVATE TARGET_LOG_NAME="Library")
target_include_directories(Library PRIVATE "${CMAKE_SOURCE_DIR}/include/Common")