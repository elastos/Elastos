
set(SAMPLE_NAME sample)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} SAMPLE_SOURCE_FILES)
add_executable(${SAMPLE_NAME} ${SAMPLE_SOURCE_FILES})
target_link_libraries(${SAMPLE_NAME} ${SPVSDK_SHARED_TARGET})
target_link_libraries(${SAMPLE_NAME} dl)

if(ANDROID)
	target_link_libraries(${SAMPLE_NAME} log atomic)
else()
	target_link_libraries(${SAMPLE_NAME} pthread)
endif()
