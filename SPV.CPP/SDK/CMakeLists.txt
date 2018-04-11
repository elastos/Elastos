
set(SPVSDK_NAME spvsdk)

############################
# Library name definitions #
############################

if(MSVC)
	set( SPV_LIBRARY_PREFIX "" )
	set( CMAKE_DEBUG_POSTFIX "d" )
else(MSVC)
	set( SPV_LIBRARY_PREFIX "lib" )
endif(MSVC)

if(APPLE)
	if(SPV_NO_LIBRARY_VERSION)
		set( SPV_LIBRARY_EXTENSION ${CMAKE_SHARED_LIBRARY_SUFFIX} )
	else(SPV_NO_LIBRARY_VERSION)
		set( SPV_LIBRARY_EXTENSION ".${SPV_ABI_SOVERSION}${CMAKE_SHARED_LIBRARY_SUFFIX}" )
	endif(SPV_NO_LIBRARY_VERSION)
elseif(WIN32)
	# Don't encode library versions on Windows systems
	set( SPV_LIBRARY_EXTENSIO ${CMAKE_SHARED_LIBRARY_SUFFIX} )
else()
	if(SPV_NO_LIBRARY_VERSION)
		set( SPV_LIBRARY_EXTENSION ${CMAKE_SHARED_LIBRARY_SUFFIX} )
	else(SPV_NO_LIBRARY_VERSION)
		set( SPV_LIBRARY_EXTENSION "${CMAKE_SHARED_LIBRARY_SUFFIX}.${SPV_ABI_SOVERSION}" )
	endif(SPV_NO_LIBRARY_VERSION)
endif()


if(NOT WIN32)
	option_with_default( SPV_BUILD_TKERNEL_WITH_INSTALL_RPATH "Build TKernel with install RPATH" ON )
	mark_as_advanced(SPV_BUILD_TKERNEL_WITH_INSTALL_RPATH)
endif()

find_package(Boost REQUIRED)
if(NOT Boost_FOUND)
	message(FATAL_ERROR "boost not found")
endif()

include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/Common"
	"${CMAKE_CURRENT_SOURCE_DIR}/../Core"
	"${CMAKE_CURRENT_SOURCE_DIR}/../Core/secp256k1"
	"${CMAKE_CURRENT_SOURCE_DIR}/../Include/spdlog/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../Include/nlohmann/single_include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../Include/Catch2/single_include"
)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../Core SPVCORE_SOURCE_FILES)
file(GLOB_RECURSE SPVSDK_SOURCE_FILES "*.cpp")
add_library(${SPVSDK_NAME} ${SPV_LIBRARY_TYPE} ${SPVSDK_SOURCE_FILES} ${SPVCORE_SOURCE_FILES})
