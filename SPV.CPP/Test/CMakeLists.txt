
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_SOURCE_FILES)
foreach(src ${TEST_SOURCE_FILES})
	string(REGEX REPLACE ".*/\(.*\).cpp$" "\\1" TEST_TARGET_NAME ${src})
	add_executable(${TEST_TARGET_NAME} ${src})
	target_link_libraries(${TEST_TARGET_NAME} spvsdk)
	target_link_libraries(${TEST_TARGET_NAME} dl)

	if(UNIX)
		if(APPLE)
			target_link_libraries(${TEST_TARGET_NAME} ${Boost_LIBRARIES})
			target_link_libraries(${TEST_TARGET_NAME} ssl crypto)
		elseif(ANDROID)
			target_link_libraries(${TEST_TARGET_NAME} log atomic)
		else()
			target_link_libraries(${TEST_TARGET_NAME} ${Boost_LIBRARIES})
			target_link_libraries(${TEST_TARGET_NAME} crypto)
		endif()
	endif()
endforeach(src)

include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/sqlite"
	"${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/Catch2/single_include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Common"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Wrapper"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Database"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDK/ELACoreExt"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDK"
	"${CMAKE_CURRENT_SOURCE_DIR}/../Core"
)
