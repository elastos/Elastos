# REQUIREMENTS: this script needs CMake 3.0 or higher. Downloads are available at http://www.cmake.org

# How to build Elastos.ELA.SPV.Cpp using CMake under Unix (Linux, Darwin etc.)?
# 1. Copy this file, as-is, to the top-level Elastos.ELA.SPV.Cpp folder
# 2. Make a build directory(Elastos.ELA.SPV.Cpp.build) somewhere outside the Elastos.ELA.SPV.Cpp source tree
# 3. cd Elastos.ELA.SPV.Cpp.build; cmake "relative/path/to/Elastos.ELA.SPV.Cpp"
# 4. make
# 5. make install (or sudo make install)

# How to build Elastos.ELA.SPV.Cpp using CMake under Windows
# 1. Copy this file, as-is, to the top-level folder
# 2. Launch the cmake-gui, and select the top-level folder
# 3. You can choose whatever you want as an install directory
# 4. Click 'Configure'
# 5. Choose your generator
# 6. When configure is achived, you can change the variable with a red background
# 7. Click 'Generate'
# 8. From your IDE, open the generated makefile and run the compilation.

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

if(SPV_FOR_ANDROID)
	message(STATUS "Build spv for android")

	set(CMAKE_SYSTEM_NAME Android)
	#set(CMAKE_ANDROID_STL_TYPE c++_static)
	set(CMAKE_ANDROID_STL_TYPE c++_shared)

	#if(NOT ANDROID_NATIVE_API_LEVEL STREQUAL "")
	#set(CMAKE_SYSTEM_VERSION ${ANDROID_NATIVE_API_LEVEL})
	#endif()

	if("$ENV{ANDROID_NDK}" STREQUAL "")
		message(FATAL_ERROR "Environment variable ANDROID_NDK not define")
	endif()
	set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK})

	#set(CMAKE_ANDROID_ARCH_ABI arm64-v8a CACHE STRING "Android ABI (architecture)")
endif()


project(SPVSDK VERSION 0.1.0.0)

if(SPV_FOR_ANDROID)
	message(STATUS "Android stl type -- ${CMAKE_ANDROID_STL_TYPE}")
	message(STATUS "Android api level -- ${CMAKE_SYSTEM_VERSION}")
	message(STATUS "Android ABI (architecture) -- ${CMAKE_ANDROID_ARCH_ABI}")
endif()

############################
# Check 32/64 bit platform #
############################
if(${CMAKE_SIZEOF_VOID_P} MATCHES "8") # It is 64bit, otherwise 32 bit systems match 4
	set(BIT 64)
else (${CMAKE_SIZEOF_VOID_P} MATCHES "8")
	set(BIT 32)
endif(${CMAKE_SIZEOF_VOID_P} MATCHES "8")
message(STATUS "Build ${BIT}bit")

if(NOT CMAKE_CONFIGURATION_TYPES)
	if(CMAKE_BUILD_TYPE STREQUAL "")
		set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type." FORCE)
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug" "RelWithDebInfo" "MinSizeRel")
	endif(CMAKE_BUILD_TYPE STREQUAL "")
endif(NOT CMAKE_CONFIGURATION_TYPES)


if(NOT DEFINED SPV_INSTALL_PREFIX)
	if(WIN32)
		set(SPV_INSTALL_PREFIX "c:/${TARGET_NAME}-${SPV_VERSION}")
	else(WIN32)
		set(SPV_INSTALL_PREFIX "/usr/local/")
	endif(WIN32)
endif(NOT DEFINED SPV_INSTALL_PREFIX)

set(SPV_INSTALL_PREFIX ${SPV_INSTALL_PREFIX} CACHE
	PATH "Install prefix prepended to target to create install location")
set(CMAKE_INSTALL_PREFIX "${SPV_INSTALL_PREFIX}" CACHE INTERNAL "Prefix jprepended to install directories if target destination is not absolute, immutable" FORCE)

option_with_default(SPV_BUILD_SHARED_LIB "Build shared SPV libs" ON)
option_with_default(SPV_BUILD_TEST_CASES "Build test cases" OFF)
option_with_default(SPV_BUILD_SAMPLE "Build sample" OFF)
option_with_default(CMAKE_EXPORT_COMPILE_COMMANDS "Export to compile_commands.json" OFF)

option_with_default(SPV_EXTRA_WARNINGS "Enable Maximum Warnings Level" OFF)

if(NOT SPV_FOR_ANDROID)
	find_package(Boost REQUIRED COMPONENTS filesystem system program_options thread)
	if(NOT Boost_FOUND)
		message(FATAL_ERROR "boost not found")
	endif()
else()
	set(BOOST_COMPONENTS filesystem system program_options thread)
	set(Boost_LIBRARIES "")

	foreach(lib_name ${BOOST_COMPONENTS})
		file(GLOB lib_name_FOUND ${CMAKE_CURRENT_SOURCE_DIR}/Boost-for-Android/build/out/${CMAKE_ANDROID_ARCH_ABI}/lib/libboost_${lib_name}*.a)
		if("${lib_name_FOUND}" STREQUAL "")
			message(FATAL_ERROR "libboost_${lib_name} not found, maybe you should run build-android.sh/build-android.bat in Boost-for-Android first.")
		endif()
		message(STATUS "${lib_name_FOUND} -- found")
		set(Boost_LIBRARIES "${lib_name_FOUND};${Boost_LIBRARIES}")
	endforeach(lib_name)

	string(REPLACE ";" " " Boost_LIBRARIES_NO_Semicolon "${Boost_LIBRARIES}")
	set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-whole-archive ${Boost_LIBRARIES_NO_Semicolon} -Wl,-no-whole-archive" CACHE STRING " falg" FORCE)
	file(GLOB Boost_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Boost-for-Android/build/out/${CMAKE_ANDROID_ARCH_ABI}/include/*)
endif()


if(SPV_BUILD_SHARED_LIB)
	set(SPV_LIBRARY_TYPE SHARED)
	set(SPV_BUILD_STATIC_LIB FALSE)
else(SPV_BUILD_SHARED_LIB)
	set(SPV_LIBRARY_TYPE STATIC)
	set(SPV_BUILD_STATIC_LIB TRUE)
	if (WIN32)
		add_definitions("-DHAVE_NO_DLL")
	endif(WIN32)
endif(SPV_BUILD_SHARED_LIB)

set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_RELEASE NDEBUG)
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_MINSIZEREL NDEBUG)
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS_RELWITHDEBINFO NDEBUG)

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
	endif()
else(UNIX)
	if(WIN32)
		######### Windows ###########
		message(STATUS "Windows system detected")
		add_definitions(-DWNT -DWIN32 -D_WINDOWS)
		set(PLATFORM win)
	else(WIN32)
		message(STATUS "Unknown platform")
	endif(WIN32)
endif(UNIX)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DSPDLOG_DEBUG_ON)
endif()

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

include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/SDK/Common"
	"${CMAKE_CURRENT_SOURCE_DIR}/SDK/Database"
	"${CMAKE_CURRENT_SOURCE_DIR}/SDK/Wrapper"
	"${CMAKE_CURRENT_SOURCE_DIR}/SDK"
	"${CMAKE_CURRENT_SOURCE_DIR}/Core/secp256k1"
	"${CMAKE_CURRENT_SOURCE_DIR}/Core"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/spdlog/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/nlohmann/single_include"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Catch2/single_include"
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${Boost_INCLUDE_DIRS}"
)


if(SPV_FOR_ANDROID)
	add_subdirectory(Java)
else()
	add_subdirectory(SDK)
endif()

if(SPV_BUILD_TEST_CASES)
	add_subdirectory(Test)
endif()

if(NOT SPV_FOR_ANDROID AND SPV_BUILD_SAMPLE)
	set(SAMPLE_NAME sample)
	aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/Sample SAMPLE_SOURCE_FILES)
	add_executable(${SAMPLE_NAME} ${SAMPLE_SOURCE_FILES})
	target_link_libraries(${SAMPLE_NAME} spvsdk)
	target_link_libraries(${SAMPLE_NAME} sqlite3)
	target_link_libraries(${SAMPLE_NAME} ${Boost_LIBRARIES})
endif()

