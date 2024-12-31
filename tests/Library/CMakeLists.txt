add_library(Library SHARED "source/main.cpp")
target_include_directories(Library PRIVATE "${CMAKE_SOURCE_DIR}/include/Common")