
set(TARGET_NAME UtilsTest)

file(GLOB_RECURSE UTILSTEST_SOURCE_FILES "*.cpp")
add_executable(${TARGET_NAME} ${UTILSTEST_SOURCE_FILES})

target_link_libraries(${TARGET_NAME} spvsdk)
target_link_libraries(${TARGET_NAME} pthread)

include_directories(
	"${CMAKE_CURRENT_SOURCE_DIR}/../Include/Catch2/single_include"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDK"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Common"
	"${CMAKE_CURRENT_SOURCE_DIR}/../SDK/Wrapper"
	"${CMAKE_CURRENT_SOURCE_DIR}/../Core"
)
