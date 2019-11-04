cmake_minimum_required(VERSION 3.10)
cmake_policy(VERSION 3.10)

execute_process(
	COMMAND git describe --tags --no-abbrev
	COMMAND tr -d "v\n"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE SPVSDK_VERSION
)

execute_process(
	COMMAND git describe --dirty --always --tags
	COMMAND tr -d "\n"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE SPVSDK_VERSION_MESSAGE
)

project(Elastos-spvsdk VERSION ${SPVSDK_VERSION})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(ProjectDefaults)
include(DistPackage)
include(AddSubModule)
if(NOT DEFINED SPDLOG_LEVEL)
	set(SPDLOG_LEVEL info CACHE STRING "Android ABI (architecture)" FORCE)
endif()
set_property(CACHE SPDLOG_LEVEL PROPERTY STRINGS trace debug info warning error critical off)

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
option(SPDSDK_DEBUG_ON "SPD debug" OFF)
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
