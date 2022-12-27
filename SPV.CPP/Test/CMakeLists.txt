project(Test)
set(CMAKE_CXX_STANDARD 11)

if (NOT SPV_ENABLE_STATIC)
    message(FATAL_ERROR "Unable to support testcase if SPV_ENABLE_STATIC is not ON")
endif()

add_custom_target(RunTestCase)

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

if(MSVC)
	add_definitions(/FI"${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Common/BRNameFix.h")
	add_definitions(/FI"${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Common/secp256k1_name_fix.h")
else()
	# GCC or Clang
	add_definitions(-include ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Common/BRNameFix.h)
	add_definitions(-include ${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Common/secp256k1_name_fix.h)
endif()

link_directories(
	${PROJECT_INT_DIST_DIR}/lib
	${CMAKE_CURRENT_BINARY_DIR}/../SDK
)

foreach(src ${TEST_SOURCE_FILES})
	string(REGEX REPLACE ".*/\(.*\).cpp$" "\\1" TEST_TARGET_NAME ${src})
	add_executable(${TEST_TARGET_NAME} ${src})

	target_link_libraries(${TEST_TARGET_NAME} spvsdk-static utf8proc.a curl.a sqlite3.a ssl.a crypto.a boost_system.a boost_filesystem.a boost_thread.a resolv z pthread dl)
	add_dependencies(${TEST_TARGET_NAME} libspvsdk catch2)

	if("${AUTO_TEST_COMMAND}" STREQUAL "")
		set(AUTO_TEST_COMMAND ./${TEST_TARGET_NAME})
	else()
		set(AUTO_TEST_COMMAND ${AUTO_TEST_COMMAND} && ./${TEST_TARGET_NAME})
	endif()
	add_dependencies(RunTestCase ${TEST_TARGET_NAME})
endforeach()

if(NOT ANDROID AND NOT IOS)
	add_custom_command(
		TARGET RunTestCase
		POST_BUILD
		COMMAND ${AUTO_TEST_COMMAND}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)
endif()
