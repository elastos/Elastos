project(Test)
set(CMAKE_CXX_STANDARD 11)

add_custom_target(
	auto_run_test ALL
)

include(ProjectDefaults)
include(CheckFunctionExists)

unset(AUTO_TEST_COMMAND)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_SOURCE_FILES)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	set(SYSTEM_LIBS pthread dl)
endif()

include_directories(
	../Interface
	../SDK
	../ThirdParty/breadwallet-core
	${CMAKE_BINARY_DIR}
	${PROJECT_INT_DIST_DIR}/include
	${PROJECT_INT_DIST_DIR}/include/catch2
)

link_directories(
	${PROJECT_INT_DIST_DIR}/lib
	${CMAKE_CURRENT_BINARY_DIR}/../SDK
)

foreach(src ${TEST_SOURCE_FILES})
	string(REGEX REPLACE ".*/\(.*\).cpp$" "\\1" TEST_TARGET_NAME ${src})
	add_executable(${TEST_TARGET_NAME} ${src})

	if(SPV_ENABLE_STATIC)
		target_link_libraries(${TEST_TARGET_NAME} spvsdk-static dl boost_filesystem boost_system boost_thread crypto ssl fruit sqlite3)
	else()
		target_link_libraries(${TEST_TARGET_NAME} spvsdk dl)
	endif()
	add_dependencies(${TEST_TARGET_NAME} libspvsdk catch2)

	if("${AUTO_TEST_COMMAND}" STREQUAL "")
		set(AUTO_TEST_COMMAND ./${TEST_TARGET_NAME})
	else()
		set(AUTO_TEST_COMMAND ${AUTO_TEST_COMMAND} && ./${TEST_TARGET_NAME})
	endif()
	add_dependencies(auto_run_test ${TEST_TARGET_NAME})
endforeach()

if(NOT ANDROID AND NOT IOS)
	add_custom_command(
		TARGET auto_run_test
		POST_BUILD
		COMMAND ${AUTO_TEST_COMMAND}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
endif()
