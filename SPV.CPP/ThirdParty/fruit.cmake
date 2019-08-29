
set(FRUIT_TESTS_USE_PRECOMPILED_HEADERS OFF CACHE INTERNAL "Don't use pre-compiled headers")

set(BOOST_DIR ${Boost_INCLUDE_DIRS} CACHE INTERNAL "boost dir")
include_directories("${BOOST_DIR}")

add_subdirectory(fruit EXCLUDE_FROM_ALL)
add_dependencies(fruit ${Boost_LIBRARIES})
set_target_properties(fruit PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(ThirdParty_FRUIT_INC_DIR
	${CMAKE_CURRENT_BINARY_DIR}/fruit/include
	${CMAKE_CURRENT_SOURCE_DIR}/fruit/include
	CACHE INTERNAL "fruit include directories")

