add_custom_target(
	auto_run_test ALL
)

unset(AUTO_TEST_COMMAND)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_SOURCE_FILES)
foreach(src ${TEST_SOURCE_FILES})
	string(REGEX REPLACE ".*/\(.*\).cpp$" "\\1" TEST_TARGET_NAME ${src})
	add_executable(${TEST_TARGET_NAME} ${src})
	target_link_libraries(${TEST_TARGET_NAME} spvsdk)
	target_link_libraries(${TEST_TARGET_NAME} dl)

	if("${AUTO_TEST_COMMAND}" STREQUAL "")
		set(AUTO_TEST_COMMAND ./${TEST_TARGET_NAME})
	else()
		set(AUTO_TEST_COMMAND ${AUTO_TEST_COMMAND} && ./${TEST_TARGET_NAME})
	endif()
	add_dependencies(auto_run_test ${TEST_TARGET_NAME})

	if(UNIX)
		if(APPLE)
			target_link_libraries(${TEST_TARGET_NAME} ${Boost_LIBRARIES})
		elseif(ANDROID)
			target_link_libraries(${TEST_TARGET_NAME} log atomic)
		else()
			target_link_libraries(${TEST_TARGET_NAME} ${Boost_LIBRARIES})
		endif()
	endif()
endforeach()

add_custom_command(
	TARGET auto_run_test
	POST_BUILD
	COMMAND ${AUTO_TEST_COMMAND}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)