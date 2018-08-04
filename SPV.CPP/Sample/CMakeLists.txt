
add_custom_command(
	COMMENT "Copy Data to Sample..."
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Data
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../Data ${CMAKE_CURRENT_BINARY_DIR}/Data
	)
add_custom_target(copy_sample_data DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Data)

set(SAMPLE_NAME sample)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SAMPLE_SOURCE_FILES)
add_executable(${SAMPLE_NAME} ${SAMPLE_SOURCE_FILES})
target_link_libraries(${SAMPLE_NAME} ${SPVSDK_SHARED_TARGET})
target_link_libraries(${SAMPLE_NAME} dl)
add_dependencies(${SAMPLE_NAME} copy_sample_data)

if(ANDROID)
	target_link_libraries(${SAMPLE_NAME} log atomic)
else()
	target_link_libraries(${SAMPLE_NAME} pthread)
endif()
