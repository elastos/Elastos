
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} TEST_SOURCE_FILES)
foreach(src ${TEST_SOURCE_FILES})
	string(REGEX REPLACE ".*/\(.*\).cpp$" "\\1" TEST_TARGET_NAME ${src})
	add_executable(${TEST_TARGET_NAME} ${src})
	target_link_libraries(${TEST_TARGET_NAME} spvsdk)
	target_link_libraries(${TEST_TARGET_NAME} pthread)
	target_link_libraries(${TEST_TARGET_NAME} sqlite3)
	#sudo apt-get install libboost-dev
	target_link_libraries(${TEST_TARGET_NAME} boost_system)
	#sudo apt-get install libboost-program-options-dev
	target_link_libraries(${TEST_TARGET_NAME} boost_program_options)
	if(APPLE)
		target_link_libraries(${TEST_TARGET_NAME} boost_thread-mt)
	else(APPLE)
		#sudo apt-get install libboost-thread-dev
		target_link_libraries(${TEST_TARGET_NAME} boost_thread)
	endif(APPLE)
endforeach(src)


include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/../Include/Catch2/single_include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDK"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Common"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Wrapper"
	"${CMAKE_CURRENT_SOURCE_DIR}/../Core"
)
