include(ProjectDefaults)

file(GLOB_RECURSE SPVSDK_SOURCE_FILES "*.cpp" "*.c")
file(GLOB_RECURSE BRETH_SOURCE_FILES
	"../ThirdParty/breadwallet-core/bitcoin/*.c"
	"../ThirdParty/breadwallet-core/ethereum/*.c"
	"../ThirdParty/breadwallet-core/support/*.c"
	"../ThirdParty/breadwallet-core/ripple/*.c")
unset(BRETH_SOURCE_FILES_REMOVE_TEST)
foreach(src ${BRETH_SOURCE_FILES})
	if (NOT src MATCHES ".*test.*.c")
		set(BRETH_SOURCE_FILES_REMOVE_TEST ${src} ${BRETH_SOURCE_FILES_REMOVE_TEST})
	endif()
endforeach()

set(SPVSDK_SOURCE_FILES ${BRETH_SOURCE_FILES_REMOVE_TEST} ${SPVSDK_SOURCE_FILES})

set(SPVSDK_DEPENDS boost libressl libsqlite json libspdlog secp256k1 libutf8proc)
set(LIBS boost_filesystem boost_system boost_thread crypto ssl sqlite3 spdlog utf8proc)

if(WIN32)
	add_definitions(
			-DWIN32_LEAN_AND_MEAN
			-D_CRT_SECURE_NO_WARNINGS
			-D_CRT_NONSTDC_NO_WARNINGS)

	# Force source code encoding to utf-8
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")

	set(SPVSDK_DEPENDS pthreads-w32 ${SPVSDK_DEPENDS})
	set(LIBS ${LIBS} pthread Ws2_32)
elseif(ANDROID)
	set(LIBS ${LIBS} log)
elseif(DARWIN OR IOS)
	set(LIBS ${LIBS} resolv)
endif()

add_custom_target(libspvsdk)

include_directories(
	.
	../ThirdParty/breadwallet-core
	../ThirdParty/breadwallet-core/include
	../Interface
	${CMAKE_CURRENT_BINARY_DIR}/..
	${CMAKE_CURRENT_BINARY_DIR}/../ThirdParty/secp256k1/${PROJECT_DEPS_BUILD_PREFIX}/src
	${CMAKE_CURRENT_BINARY_DIR}/../ThirdParty/secp256k1/${PROJECT_DEPS_BUILD_PREFIX}/src/secp256k1
	${CMAKE_CURRENT_BINARY_DIR}/../ThirdParty/secp256k1/${PROJECT_DEPS_BUILD_PREFIX}/src/secp256k1/src
	${PROJECT_INT_DIST_DIR}/include)

link_directories(
	${PROJECT_INT_DIST_DIR}/lib)

if(MSVC)
	add_definitions(/FI"${CMAKE_CURRENT_SOURCE_DIR}/Common/BRNameFix.h")
	add_definitions(/FI"${CMAKE_CURRENT_SOURCE_DIR}/Common/secp256k1_name_fix.h")
else()
	# GCC or Clang
	add_definitions(-include ${CMAKE_CURRENT_SOURCE_DIR}/Common/BRNameFix.h)
	add_definitions(-include ${CMAKE_CURRENT_SOURCE_DIR}/Common/secp256k1_name_fix.h)
endif()

if(SPV_ENABLE_STATIC)
	add_library(spvsdk-static STATIC ${SPVSDK_SOURCE_FILES})
	add_dependencies(spvsdk-static ${SPVSDK_DEPENDS})

	set_target_properties(spvsdk-static PROPERTIES OUTPUT_NAME spvsdk)

	add_dependencies(libspvsdk spvsdk-static)
	install(TARGETS spvsdk-static
		RUNTIME DESTINATION "bin"
		ARCHIVE DESTINATION "lib"
		LIBRARY DESTINATION "lib")
endif()

if(SPV_ENABLE_SHARED)
	add_library(spvsdk-shared SHARED ${SPVSDK_SOURCE_FILES})
	add_dependencies(spvsdk-shared ${SPVSDK_DEPENDS})

	set_target_properties(spvsdk-shared PROPERTIES
		OUTPUT_NAME spvsdk
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR})

	target_link_libraries(spvsdk-shared PRIVATE ${LIBS})

	add_dependencies(libspvsdk spvsdk-shared)
	install(TARGETS spvsdk-shared
		RUNTIME DESTINATION "bin"
		ARCHIVE DESTINATION "lib"
		LIBRARY DESTINATION "lib")
endif()

file(GLOB_RECURSE SPVSDK_HEADERS "../Interface/*.h")

install(FILES ${SPVSDK_HEADERS} DESTINATION "include")
install(FILES ${PROJECT_INT_DIST_DIR}/include/nlohmann/json.hpp DESTINATION "include/nlohmann")
