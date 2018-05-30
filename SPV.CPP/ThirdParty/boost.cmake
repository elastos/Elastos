
set(BOOST_BUILD_COMPONENTS filesystem system program_options thread locale)

if(SPV_FOR_ANDROID)
	set(Boost_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/Boost-for-Android/build/out/${CMAKE_ANDROID_ARCH_ABI}/include)
	unset(COMPONENTS_LIBRARY_PATH)
	unset(Boost_LIBRARIES)
	foreach(COMPONENT_NAME ${BOOST_BUILD_COMPONENTS})
		set(COMPONENTS_LIBRARY_PATH ${COMPONENTS_LIBRARY_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/Boost-for-Android/build/out/${CMAKE_ANDROID_ARCH_ABI}/lib/libboost_${COMPONENT_NAME}.a)
		set(Boost_LIBRARIES boost_${COMPONENT_NAME};${Boost_LIBRARIES})
	endforeach()

	unset(BUILD_WITH_LIBRARIES)
	string(REPLACE ";" "," BUILD_WITH_LIBRARIES "${BOOST_BUILD_COMPONENTS}")
	add_custom_command(
		COMMENT "Building boost..."
		OUTPUT ${COMPONENTS_LIBRARY_PATH}
		COMMAND ./build-android.sh --with-libraries=${BUILD_WITH_LIBRARIES} --arch=${CMAKE_ANDROID_ARCH_ABI} $ENV{ANDROID_NDK}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Boost-for-Android
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Boost-for-Android/build-android.sh
	)

	add_custom_target(build_boost ALL DEPENDS ${COMPONENTS_LIBRARY_PATH})

	foreach(COMPONENT_NAME ${BOOST_BUILD_COMPONENTS})
		add_library(boost_${COMPONENT_NAME} STATIC IMPORTED GLOBAL)
		set(COMPONENT_LIBRARY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Boost-for-Android/build/out/${CMAKE_ANDROID_ARCH_ABI}/lib/libboost_${COMPONENT_NAME}.a)
		set_target_properties(boost_${COMPONENT_NAME} PROPERTIES IMPORTED_LOCATION ${COMPONENT_LIBRARY_PATH})
		add_dependencies(boost_${COMPONENT_NAME} build_boost)
	endforeach()
else()
	find_package(Boost REQUIRED COMPONENTS ${BOOST_BUILD_COMPONENTS})
	if(NOT Boost_FOUND)
		message(FATAL_ERROR "boost not found")
	endif()
endif()

set(
	Boost_LIBRARIES
	${Boost_LIBRARIES}
	CACHE INTERNAL "boost libraries" FORCE
)
set(
	ThirdParty_BOOST_INC_DIR
	${Boost_INCLUDE_DIRS}
	CACHE INTERNAL "boost include directory" FORCE
)