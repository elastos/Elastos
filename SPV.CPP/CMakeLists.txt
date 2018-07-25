cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define helper macro option_with_default
macro(option_with_default OPTION_NAME OPTION_STRING OPTION_DEFAULT)
	if(NOT DEFINED ${OPTION_NAME})
		set(${OPTION_NAME} ${OPTION_DEFAULT})
	endif(NOT DEFINED ${OPTION_NAME})
	option(${OPTION_NAME} "${OPTION_STRING}" ${${OPTION_NAME}})
endmacro(option_with_default OPTION_NAME OPTION_STRING OPTION_DEFAULT)

option_with_default(SPV_FOR_ANDROID "Build spv for android" OFF)
option_with_default(SPV_EXPORT_JNI "SPV export java native interface for android" OFF)

if(SPV_FOR_ANDROID)
	message(STATUS "Build spv for android")

	set(CMAKE_SYSTEM_NAME Android)
	set(CMAKE_ANDROID_STL_TYPE c++_static)
	#set(CMAKE_ANDROID_STL_TYPE c++_shared)

	if(NOT DEFINED ANDROID_NATIVE_API_LEVEL)
		set(CMAKE_SYSTEM_VERSION 23)
	else()
		set(CMAKE_SYSTEM_VERSION ${ANDROID_NATIVE_API_LEVEL})
	endif()

	if("$ENV{ANDROID_NDK}" STREQUAL "")
		message(FATAL_ERROR "Environment variable ANDROID_NDK not define")
	endif()
	set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK})
	set(CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION clang)

	# can be value below: arm64-v8a armeabi-v7a armeabi mips mips64 x86 x86_64
	if(NOT DEFINED CMAKE_ANDROID_ARCH_ABI)
		set(CMAKE_ANDROID_ARCH_ABI armeabi-v7a CACHE STRING "Android ABI (architecture)" FORCE)
	endif()
	set_property(CACHE CMAKE_ANDROID_ARCH_ABI PROPERTY STRINGS arm64-v8a armeabi-v7a armeabi mips mips64 x86 x86_64)
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type." FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Release Debug RelWithDebInfo MinSizeRel)


project(SPVSDK)


#set(CMAKE_VERBOSE_MAKEFILE ON)

execute_process(
	COMMAND git describe --dirty --always --tags
	COMMAND tr -d "\n"
	OUTPUT_VARIABLE SPVSDK_VERSION_MESSAGE
)

if(NOT DEFINED SPDLOG_LEVEL)
	set(SPDLOG_LEVEL debug CACHE STRING "Android ABI (architecture)" FORCE)
endif()
set_property(CACHE SPDLOG_LEVEL PROPERTY STRINGS trace debug info warning error critical off)

if(SPV_FOR_ANDROID)
	message(STATUS "Android stl type -- ${CMAKE_ANDROID_STL_TYPE}")
	message(STATUS "Android api level -- ${CMAKE_SYSTEM_VERSION}")
	message(STATUS "Android ABI (architecture) -- ${CMAKE_ANDROID_ARCH_ABI}")
endif()

if(UNIX)
	if(APPLE)
		######### MacOSX ###########
		message(STATUS "MacOSX platform detected")
		set(PLATFORM Darwin)
	elseif(ANDROID)
		message(STATUS "Android platform detected")
		set(PLATFORM Android)
	else()
		######### Unix/Linux ###########
		message(STATUS "Unix/Linux platform detected")
		set(PLATFORM Unix)
		#show gcc/g++ compile info
		#set(CMAKE_VERBOSE_MAKEFILE ON)
#		if(CMAKE_COMPILER_IS_GNUCXX)
#			add_definitions("-fpermissive")
#		endif(CMAKE_COMPILER_IS_GNUCXX)
	endif()
else(UNIX)
	if(WIN32)
		######### Windows ###########
		message(STATUS "Windows system detected")
		add_definitions(-DWNT -DWIN32 -D_WINDOWS)
		set(PLATFORM win)
	else(WIN32)
		message(FATAL_ERROR "Unknown platform")
	endif(WIN32)
endif(UNIX)

# Enable Extra warnings for each compiler
if(SPV_EXTRA_WARNINGS)
	if(MSVC)
		add_definitions("/W4 /wd4100 /wd4206 /wd4127")
	endif(MSVC)
	if(CMAKE_COMPILER_IS_GNUCXX)
		add_definitions("-Wall -Wextra")
	endif(CMAKE_COMPILER_IS_GNUCXX)
	if(CLANG)
		add_definitions("-Weverything")
	endif(CLANG)
	if(INTEL)
		add_definitions("-Wall -Wremarks")
	endif(INTEL)
endif(SPV_EXTRA_WARNINGS)

if(${CMAKE_SIZEOF_VOID_P} MATCHES "8") # It is 64bit, otherwise 32 bit systems match 4
	set(BIT 64)
else (${CMAKE_SIZEOF_VOID_P} MATCHES "8")
	set(BIT 32)
endif(${CMAKE_SIZEOF_VOID_P} MATCHES "8")
message(STATUS "Build ${BIT}bit")

if(NOT DEFINED SPV_INSTALL_PREFIX)
	if(WIN32)
		set(SPV_INSTALL_PREFIX "c:/${TARGET_NAME}")
	else(WIN32)
		set(SPV_INSTALL_PREFIX "/usr/local/")
	endif(WIN32)
endif(NOT DEFINED SPV_INSTALL_PREFIX)

set(
	SPV_INSTALL_PREFIX
	${SPV_INSTALL_PREFIX}
	CACHE PATH "Install prefix prepended to target to create install location"
)

set(
	CMAKE_INSTALL_PREFIX
	${SPV_INSTALL_PREFIX}
	CACHE INTERNAL "Prefix to install" FORCE
)

option_with_default(SPV_BUILD_SHARED_LIB "Build shared SPV libs" ON)
option_with_default(SPV_BUILD_TEST_CASES "Build test cases" OFF)
option_with_default(SPV_BUILD_SAMPLE "Build sample" OFF)
option_with_default(CMAKE_EXPORT_COMPILE_COMMANDS "Export to compile_commands.json" OFF)

option_with_default(SPV_EXTRA_WARNINGS "Enable Maximum Warnings Level" OFF)

set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_RELEASE NDEBUG)
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL NDEBUG)
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO NDEBUG)

if(SPV_BUILD_SHARED_LIB)
	set(SPV_LIBRARY_TYPE SHARED)
else(SPV_BUILD_SHARED_LIB)
	set(SPV_LIBRARY_TYPE STATIC)
endif(SPV_BUILD_SHARED_LIB)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DSPDLOG_DEBUG_ON)
	add_definitions(-DSPDLOG_TRACE_ON)
endif()

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/Config.h"
)

add_subdirectory(ThirdParty)
add_subdirectory(Core)
add_subdirectory(SDK)
add_subdirectory(Interface)
if(SPV_FOR_ANDROID AND SPV_EXPORT_JNI)
	add_subdirectory(Java)
endif()

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${ThirdParty_INC_DIRS}
	${SPVCORE_INC_DIRS}
	${SPVSDK_INC_DIRS}
	${SPVJNI_INC_DIRS}
	${INTERFACE_INC_DIRS}
)

add_library(spvsdk ${SPV_LIBRARY_TYPE}
	${SPVSDK_SOURCE_FILES}
	${SPVJNI_SOURCE_FILES}
	${SPVCORE_SOURCE_FILES}
)

target_link_libraries(spvsdk dl ${Boost_LIBRARIES} ${OpenSSL_LIBRARIES} sqlite bigint)
if(ANDROID)
	target_link_libraries(spvsdk log atomic)
endif()
add_dependencies(spvsdk ${Boost_LIBRARIES} ${OpenSSL_LIBRARIES} sqlite bigint)

if(SPV_BUILD_TEST_CASES)
	add_subdirectory(Test)
endif()

if(SPV_BUILD_SAMPLE)
	add_subdirectory(Sample)
endif()
