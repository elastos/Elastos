include(ProjectDefaults)

file(GLOB_RECURSE SPVSDK_SOURCE_FILES "*.cpp" "*.c")
file(GLOB_RECURSE BRETH_SOURCE_FILES
	"../ThirdParty/breadwallet-core/bitcoin/*.c"
	"../ThirdParty/breadwallet-core/ethereum/*.c"
	"../ThirdParty/breadwallet-core/support/*.c")
unset(BRETH_SOURCE_FILES_REMOVE_TEST)
foreach(src ${BRETH_SOURCE_FILES})
	if (NOT src MATCHES ".*test.*.c")
		set(BRETH_SOURCE_FILES_REMOVE_TEST ${src} ${BRETH_SOURCE_FILES_REMOVE_TEST})
	endif()
endforeach()

set(SPVSDK_SOURCE_FILES ${BRETH_SOURCE_FILES_REMOVE_TEST} ${SPVSDK_SOURCE_FILES})

set(SPVSDK_DEPENDS boost libressl libfruit libsqlite json libspdlog secp256k1)
set(LIBS boost_filesystem boost_system boost_thread crypto ssl fruit sqlite3 spdlog)
if(ANDROID)
	set(LIBS ${LIBS} log)
else()
	set(LIBS ${LIBS} resolv)
endif()

add_custom_target(libspvsdk)

include_directories(
	.
	../ThirdParty/breadwallet-core
	../Interface
	${CMAKE_CURRENT_BINARY_DIR}/..
	${CMAKE_CURRENT_BINARY_DIR}/../ThirdParty/secp256k1/${PROJECT_DEPS_BUILD_PREFIX}/src
	${CMAKE_CURRENT_BINARY_DIR}/../ThirdParty/secp256k1/${PROJECT_DEPS_BUILD_PREFIX}/src/secp256k1
	${CMAKE_CURRENT_BINARY_DIR}/../ThirdParty/secp256k1/${PROJECT_DEPS_BUILD_PREFIX}/src/secp256k1/src
	${PROJECT_INT_DIST_DIR}/include)

link_directories(
	${PROJECT_INT_DIST_DIR}/lib)

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
