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

option_with_default(SPV_EXPORT_JNI "SPV export java native interface for android" OFF)
set(SPV_PLATFORM_ANDROID "Android" CACHE INTERNAL "SPV platform" FORCE)
set(SPV_PLATFORM_IOS "IOS" CACHE INTERNAL "SPV platform" FORCE)

if(NOT DEFINED SPV_PLATFORM)
	set(SPV_PLATFORM "Host" CACHE STRING "SPV platform" FORCE)
endif()
set_property(CACHE SPV_PLATFORM PROPERTY STRINGS ${SPV_PLATFORM_ANDROID} ${SPV_PLATFORM_IOS} Host)

if(${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_ANDROID})
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

	# can be value below: arm64-v8a armeabi-v7a armeabi x86 x86_64
	if(NOT DEFINED CMAKE_ANDROID_ARCH_ABI)
		set(CMAKE_ANDROID_ARCH_ABI armeabi-v7a CACHE STRING "Android ABI (architecture)" FORCE)
	endif()
	set_property(CACHE CMAKE_ANDROID_ARCH_ABI PROPERTY STRINGS arm64-v8a armeabi-v7a armeabi x86 x86_64)
elseif(${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_IOS})
	message(STATUS "Build spv for IOS")
	set(IOS_DEPLOYMENT_TARGET 10.0)
	set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/platforms/ios/cmake/Toolchains/iOS.cmake)
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type." FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Release Debug RelWithDebInfo MinSizeRel)

if(${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_IOS})
	option(BUILD_SHARED_LIBS "SPV build shared library." OFF)
	if(BUILD_SHARED_LIBS)
		message(FATAL_ERROR "SPV not support shared library for IOS.")
	endif()
else()
	option(BUILD_SHARED_LIBS "SPV build shared library." ON)
endif()
message(STATUS "SPV build shared library -- ${BUILD_SHARED_LIBS}")


project(SPVSDK)


#set(CMAKE_VERBOSE_MAKEFILE ON)

execute_process(
	COMMAND git describe --dirty --always --tags
	COMMAND tr -d "\n"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE SPVSDK_VERSION_MESSAGE
)

if(NOT DEFINED SPDLOG_LEVEL)
	set(SPDLOG_LEVEL info CACHE STRING "Android ABI (architecture)" FORCE)
endif()
set_property(CACHE SPDLOG_LEVEL PROPERTY STRINGS trace debug info warning error critical off)

if(${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_ANDROID})
	message(STATUS "Android stl type -- ${CMAKE_ANDROID_STL_TYPE}")
	message(STATUS "Android api level -- ${CMAKE_SYSTEM_VERSION}")
	message(STATUS "Android ABI (architecture) -- ${CMAKE_ANDROID_ARCH_ABI}")
endif()

# Enable Extra warnings for each compiler
if(SPV_EXTRA_WARNINGS)
	if(CMAKE_COMPILER_IS_GNUCXX)
		add_definitions("-Wall -Wextra")
	endif(CMAKE_COMPILER_IS_GNUCXX)
	if(CLANG)
		add_definitions("-Weverything")
	endif(CLANG)
endif(SPV_EXTRA_WARNINGS)

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
	set(CMAKE_INSTALL_PREFIX "/usr/local/" CACHE PATH "Install prefix prepended to target to create install location")
endif(NOT DEFINED CMAKE_INSTALL_PREFIX)

option_with_default(SPV_BUILD_TEST_CASES "Build test cases" OFF)
option_with_default(SPV_BUILD_SAMPLE "Build sample" OFF)
option_with_default(CMAKE_EXPORT_COMPILE_COMMANDS "Export to compile_commands.json" OFF)

option_with_default(SPV_EXTRA_WARNINGS "Enable Maximum Warnings Level" OFF)

set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_RELEASE NDEBUG)
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL NDEBUG)
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO NDEBUG)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DSPDLOG_DEBUG_ON)
	add_definitions(-DSPDLOG_TRACE_ON)
endif()

set(
	OTHER_INC_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	CACHE INTERNAL "config files" FORCE
)
configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/Config.h
)

add_subdirectory(ThirdParty)
add_subdirectory(Core)
add_subdirectory(SDK)
add_subdirectory(Interface)
if(${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_ANDROID} AND SPV_EXPORT_JNI)
	add_subdirectory(Java)
endif()

include_directories(
	${OTHER_INC_DIRS}
	${ThirdParty_INC_DIRS}
	${SPVCORE_INC_DIRS}
	${SPVSDK_INC_DIRS}
	${SPVJNI_INC_DIRS}
	${INTERFACE_INC_DIRS}
	${FRUIT_INC_DIRS}
)

set(SPVSDK_OBJECT_TARGET spvsdk_object)

add_library(${SPVSDK_OBJECT_TARGET} OBJECT
	${SPVSDK_SOURCE_FILES}
	${SPVJNI_SOURCE_FILES}
	${SPVCORE_SOURCE_FILES}
)
add_dependencies(${SPVSDK_OBJECT_TARGET} ${Boost_LIBRARIES} ${OpenSSL_LIBRARIES} sqlite bigint)
set_target_properties(${SPVSDK_OBJECT_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(spvsdk $<TARGET_OBJECTS:${SPVSDK_OBJECT_TARGET}>)
set_target_properties(spvsdk PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(spvsdk dl ${Boost_LIBRARIES} ${OpenSSL_LIBRARIES} sqlite bigint)
if(ANDROID)
	target_link_libraries(spvsdk log atomic)
endif()

if(SPV_BUILD_TEST_CASES)
	add_subdirectory(Test)
endif()

if(SPV_BUILD_SAMPLE)
	add_subdirectory(Sample)
endif()

if(UNIX AND NOT BUILD_SHARED_LIBS)
	unset(ALL_LIBRARIES)
	unset(ALL_ARCHS)
	unset(LIBTOOL_PATH)
	unset(AR_PATH)

	if(CMAKE_VERBOSE_MAKEFILE)
		set(COMBIN_VERBOSE --verbose)
	else()
		unset(COMBIN_VERBOSE)
	endif()

	set(ALL_LIBRARIES $<TARGET_FILE:spvsdk>;$<TARGET_FILE:sqlite>;$<TARGET_FILE:bigint>)
	foreach(Boost_Library ${Boost_LIBRARIES})
		set(ALL_LIBRARIES ${ALL_LIBRARIES};$<TARGET_FILE:${Boost_Library}>)
	endforeach()
	foreach(OpenSSL_Library ${OpenSSL_LIBRARIES})
		set(ALL_LIBRARIES ${ALL_LIBRARIES};$<TARGET_FILE:${OpenSSL_Library}>)
	endforeach()

	string(REPLACE ";" "," ALL_LIBRARIES "${ALL_LIBRARIES}")
	if(${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_IOS})
		set(LIBTOOL_PATH ${IOS_LIBTOOL})
		set(AR_PATH ${IOS_AR})
		string(REPLACE ";" "," ALL_ARCHS "${IOS_ARCH}")
	elseif(${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_ANDROID})
		set(AR_PATH ${CMAKE_AR})
		set(ALL_ARCHS ${CMAKE_ANDROID_ARCH_ABI})
	else()
		set(AR_PATH ${CMAKE_AR})
		execute_process(COMMAND xcrun -find libtool
			OUTPUT_VARIABLE LIBTOOL_PATH
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		set(ALL_ARCHS ${CMAKE_HOST_SYSTEM_PROCESSOR})
	endif()

	set(SPVSDK_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/lib)
	set(COMBIN_STATIC_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/platforms/scripts/combin_static_libraries.sh)

	set(
		COMPOSE_COMMAND
		${COMBIN_STATIC_SCRIPT}
		--libs=${ALL_LIBRARIES}
		--archs=${ALL_ARCHS}
		--system-name=${CMAKE_SYSTEM_NAME}
		--output-dir=${SPVSDK_OUTPUT_DIR}
		--output-filename=libspvsdk.a
		--libtool=${LIBTOOL_PATH}
		--ar-path=${AR_PATH}
		--ranlib-path=${CMAKE_RANLIB}
		${COMBIN_VERBOSE}
	)

	add_custom_command(
		COMMENT "Generating a fat uber library..."
		OUTPUT ${SPVSDK_OUTPUT_DIR}/libspvsdk.a
		COMMAND ${COMPOSE_COMMAND}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		DEPENDS spvsdk
	)

	add_custom_target(spvsdk_combin_static ALL DEPENDS ${SPVSDK_OUTPUT_DIR}/libspvsdk.a)
	add_library(spvsdk_static_uber STATIC IMPORTED GLOBAL)
	set_target_properties(spvsdk_static_uber PROPERTIES IMPORTED_LOCATION ${SPVSDK_OUTPUT_DIR}/libspvsdk.a)
	add_dependencies(spvsdk_static_uber spvsdk_combin_static)
endif()

file(GLOB INSTALL_HEADER_FILES "Interface/*.h")

if(BUILD_SHARED_LIBS)
	install(TARGETS spvsdk LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
else()
	install(FILES ${SPVSDK_OUTPUT_DIR}/libspvsdk.a DESTINATION lib)
endif()
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Data DESTINATION .)
install(FILES ${INSTALL_HEADER_FILES} DESTINATION include)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/Interface/nlohmann/json.hpp DESTINATION include/nlohmann)

