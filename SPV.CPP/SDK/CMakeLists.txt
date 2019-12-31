include(ProjectDefaults)

file(GLOB_RECURSE SPVSDK_SOURCE_FILES "*.cpp" "*.c")
aux_source_directory(../Core/bcash SPVCORE_BCASH_SOURCE_FILES)
aux_source_directory(../Core/ SPVCORE_SOURCE_FILES)
set(SPVSDK_SOURCE_FILES
	${SPVSDK_SOURCE_FILES}
	${SPVCORE_BCASH_SOURCE_FILES}
	${SPVCORE_SOURCE_FILES})

set(SPVSDK_DEPENDS boost libressl libfruit libsqlite json libspdlog)
set(LIBS boost_filesystem boost_system boost_thread crypto ssl fruit sqlite3 spdlog)
if(ANDROID)
	set(LIBS ${LIBS} log)
endif()

add_custom_target(libspvsdk)

include_directories(
	.
	../Core
	../Interface
	${CMAKE_CURRENT_BINARY_DIR}/..
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

set(SPVSDK_HEADERS
	../Interface/IIDChainSubWallet.h
	../Interface/IMainchainSubWallet.h
	../Interface/IMasterWallet.h
	../Interface/IMasterWalletManager.h
	../Interface/ISidechainSubWallet.h
	../Interface/ISubWallet.h
	../Interface/ISubWalletCallback.h
	../Interface/ITokenchainSubWallet.h
	../Interface/MasterWalletManager.h)

install(FILES ${SPVSDK_HEADERS} DESTINATION "include")
install(FILES ${PROJECT_INT_DIST_DIR}/include/nlohmann/json.hpp DESTINATION "include/nlohmann")
