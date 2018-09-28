
set(SECP256K1_ROOT_DIR    ${CMAKE_CURRENT_SOURCE_DIR}/secp256k1)
set(SECP256K1_BUILD_DIR   ${CMAKE_CURRENT_BINARY_DIR}/secp256k1/build)
set(SECP256K1_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/secp256k1/install)

file(MAKE_DIRECTORY ${SECP256K1_BUILD_DIR})

if(SPV_FOR_ANDROID)
	set(
		SECP256K1_BUILD_COMMAND
		./build.sh
		--verbose
		--build-dir=${SECP256K1_BUILD_DIR}
		--prefix=${SECP256K1_INSTALL_DIR}
		--arch=${CMAKE_ANDROID_ARCH_ABI}
		--ndk-root=$ENV{ANDROID_NDK}
		--ndk-api-level=${CMAKE_SYSTEM_VERSION}
	)
	set(SECP256K1_LIBRARY ${SECP256K1_INSTALL_DIR}/${CMAKE_ANDROID_ARCH_ABI}/lib/libsecp256k1.a)
	set(SECP256K1_INCLUDE_DIR ${SECP256K1_INSTALL_DIR}/${CMAKE_ANDROID_ARCH_ABI}/include)
else()
	set(SECP256K1_LIBRARY ${SECP256K1_INSTALL_DIR}/lib/libsecp256k1.a)
	set(SECP256K1_INCLUDE_DIR ${SECP256K1_INSTALL_DIR}/include)
endif()

add_custom_command(
	COMMENT "Auto generate secp256k1..."
	OUTPUT ${SECP256K1_ROOT_DIR}/configure
	COMMAND ./autogen.sh
	WORKING_DIRECTORY ${SECP256K1_ROOT_DIR}
)

add_custom_command(
	COMMENT "Building secp256k1..."
	OUTPUT ${SECP256K1_LIBRARY}
	COMMAND ${SECP256K1_ROOT_DIR}/configure --prefix=${SECP256K1_INSTALL_DIR}
	COMMAND make && make install
	WORKING_DIRECTORY ${SECP256K1_BUILD_DIR}
)

add_custom_target(autogen_secp256k1 DEPENDS ${SECP256K1_ROOT_DIR}/configure)
add_custom_target(build_secp256k1 DEPENDS ${SECP256K1_LIBRARY})

add_library(secp256k1 STATIC IMPORTED GLOBAL)
add_dependencies(secp256k1 build_secp256k1)
add_dependencies(build_secp256k1 autogen_secp256k1)

set_target_properties(secp256k1 PROPERTIES IMPORTED_LOCATION ${SECP256K1_LIBRARY})

set(
	ThirdParty_SECP256K1_INC_DIR
	${SECP256K1_INCLUDE_DIR}
	${SECP256K1_ROOT_DIR}
	CACHE INTERNAL "secp256k1 include directory" FORCE
)

