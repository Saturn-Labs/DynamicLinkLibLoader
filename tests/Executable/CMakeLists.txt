add_executable(Executable "source/main.cpp")
add_test(NAME Executable COMMAND Executable)
add_dependencies(Executable DynamicLinkLibLoader)
target_include_directories(Executable PRIVATE "${CMAKE_SOURCE_DIR}/include/Common")