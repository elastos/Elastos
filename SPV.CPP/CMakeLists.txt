cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)

execute_process(
	COMMAND git describe --tags --no-abbrev --exclude "*-pre"
	COMMAND tr -d "v\n"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE SPVSDK_VERSION
)

execute_process(
	COMMAND git describe --dirty --always --tags --exclude "*-pre"
	COMMAND tr -d "\n"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE SPVSDK_VERSION_MESSAGE
)

project(Elastos-spvsdk VERSION ${SPVSDK_VERSION})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
include(ProjectDefaults)
include(DistPackage)
include(AddSubModule)

set(LOG_LEVEL
	SPDLOG_LEVEL_TRACE
	SPDLOG_LEVEL_DEBUG
	SPDLOG_LEVEL_INFO
	SPDLOG_LEVEL_WARN
	SPDLOG_LEVEL_ERROR
	SPDLOG_LEVEL_CRITICAL
	SPDLOG_LEVEL_OFF)

if(NOT DEFINED SPVLOG_LEVEL)
	set(SPVLOG_LEVEL SPDLOG_LEVEL_INFO CACHE STRING "spv log level" FORCE)
endif()
set_property(CACHE SPVLOG_LEVEL PROPERTY STRINGS ${LOG_LEVEL})

if(NOT DEFINED SPDLOG_ACTIVE_LEVEL)
	set(SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO CACHE STRING "spdlog active level" FORCE)
endif()
set_property(CACHE SPDLOG_ACTIVE_LEVEL PROPERTY STRINGS ${LOG_LEVEL})

if(WIN32)
	set(ENABLE_SHARED_DEFAULT TRUE)
	set(ENABLE_STATIC_DEFAULT FALSE)
else()
	set(ENABLE_SHARED_DEFAULT TRUE)
	set(ENABLE_STATIC_DEFAULT TRUE)
endif()

option(SPV_ENABLE_SHARED "Build shared library" ${ENABLE_SHARED_DEFAULT})
option(SPV_ENABLE_STATIC "Build static library" ${ENABLE_STATIC_DEFAULT})
option(ARGUMENT_LOG_ENABLE "Eenable print argument that caller pass through" ON)
option(SPV_CONSOLE_LOG "Enable console log" OFF)
option(SPV_BUILD_TEST_CASES "Build test cases" OFF)
option(SPV_BUILD_APPS "Build command line elawallet" ON)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeConfig.h.in
	${CMAKE_CURRENT_BINARY_DIR}/CMakeConfig.h
)

add_subdirectory(ThirdParty)
add_subdirectory(SDK)

if(SPV_BUILD_APPS)
	add_subdirectory(Wallet)
endif()

if(SPV_BUILD_TEST_CASES)
	add_subdirectory(Test)
endif()

message(STATUS "version ${SPVSDK_VERSION_MESSAGE}")
