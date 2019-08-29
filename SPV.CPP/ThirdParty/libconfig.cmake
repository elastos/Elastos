
option(BUILD_EXAMPLES "Enable examples" OFF)
option(BUILD_TESTS "Enable tests" OFF)

add_subdirectory(libconfig)
set_target_properties(config++ PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(ThirdParty_LIBCONFIG_INC_DIR
	${CMAKE_CURRENT_SOURCE_DIR}/libconfig/lib
	CACHE INTERNAL "libconfig include directories")