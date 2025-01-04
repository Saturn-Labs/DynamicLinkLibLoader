add_library(Library SHARED "source/main.cpp")
target_compile_definitions(Library PRIVATE TARGET_LOG_NAME="Library")
target_include_directories(Library PRIVATE "${CMAKE_SOURCE_DIR}/include/Common")
file(GLOB_RECURSE LIBRARIES "libraries/**.lib")
file(GLOB_RECURSE LINKS "links/**.link.json")
get_target_property(TARGET_OUTPUT_FILENAME Library OUTPUT_NAME)
if (NOT TARGET_OUTPUT_FILENAME)
	set(TARGET_OUTPUT_FILENAME Library)
endif()

if(WIN32)
	set(TARGET_OUTPUT_FILENAME "${TARGET_OUTPUT_FILENAME}.dll")
endif()

foreach(LINK ${LINKS})
	set(TARGET_MODLNK_FILE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER}}/${TARGET_OUTPUT_FILENAME}")
	add_custom_command(TARGET Library PRE_BUILD
		COMMAND "dynalinker" "-input" "${LINK}" "-output" "${CMAKE_CURRENT_SOURCE_DIR}/libraries" "-version" "1.0.0" "-generatelib" "${ARCH_NAME}"
	)

	add_custom_command(TARGET Library POST_BUILD
		COMMAND "dynalinker" "-input" "${LINK}" "-output" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER}}" "-version" "1.0.0" "-modulepatch" "${TARGET_MODLNK_FILE}" "-entryredirect"
	)
endforeach()
target_link_libraries(Library PRIVATE ${LIBRARIES})