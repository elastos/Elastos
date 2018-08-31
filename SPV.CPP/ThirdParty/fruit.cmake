
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "build shared library")
set(FRUIT_TESTS_USE_PRECOMPILED_HEADERS OFF CACHE INTERNAL "Don't use pre-compiled headers")

if(SPV_FOR_ANDROID)
	set(BOOST_DIR ${BOOST_INSTALL_DIR}/${CMAKE_ANDROID_ARCH_ABI}/include CACHE INTERNAL "boost dir")
else()
	set(BOOST_DIR ${BOOST_INSTALL_DIR}/include CACHE INTERNAL "boost dir")
endif()
include_directories("${BOOST_DIR}")

add_subdirectory(fruit EXCLUDE_FROM_ALL)
add_dependencies(fruit ${Boost_LIBRARIES})

set(ThirdParty_FRUIT_INC_DIRS
	${CMAKE_CURRENT_BINARY_DIR}/fruit/include
	${CMAKE_CURRENT_SOURCE_DIR}/fruit/include
	CACHE INTERNAL "fruit include directories")

