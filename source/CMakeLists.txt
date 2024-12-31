add_library(DynamicLinkLibLoader SHARED
    main.cpp
)

target_include_directories(DynamicLinkLibLoader
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include
)