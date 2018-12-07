
set(OPENSSL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/openssl)
set(OPENSSL_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl/build)
set(OPENSSL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl/install)
set(OPENSSL_BUILD_WORKING_DIR ${OPENSSL_ROOT_DIR})

file(MAKE_DIRECTORY ${OPENSSL_BUILD_DIR})

if(${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_ANDROID})
	set(OPENSSL_BUILD_COMMENT "Building openssl for android...")
	set(OPENSSL_SELECT_VERSION cd ${OPENSSL_ROOT_DIR}/openssl && git reset --hard origin/master || echo Never mind)
	set(
		OPENSSL_BUILD_COMMAND
		./build.sh
		--verbose
		--build-dir=${OPENSSL_BUILD_DIR}
		--prefix=${OPENSSL_INSTALL_DIR}
		--arch=${CMAKE_ANDROID_ARCH_ABI}
		--ndk-root=$ENV{ANDROID_NDK}
		--ndk-api-level=${CMAKE_SYSTEM_VERSION}
		)
	set(OPENSSL_SSL_LIBRARY ${OPENSSL_INSTALL_DIR}/${CMAKE_ANDROID_ARCH_ABI}/lib/libssl.a)
	set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_INSTALL_DIR}/${CMAKE_ANDROID_ARCH_ABI}/lib/libcrypto.a)
	set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL_DIR}/${CMAKE_ANDROID_ARCH_ABI}/include)
elseif(${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_IOS})
	set(OPENSSL_BUILD_COMMENT "Building openssl for ios...")
	set(OPENSSL_SELECT_VERSION git submodule update --force || echo Nerver mind)
	set(
		OPENSSL_BUILD_COMMAND
		./build-libssl.sh
		--version=1.1.0h
	)
	set(OPENSSL_BUILD_WORKING_DIR ${CMAKE_CURRENT_SOURCE_DIR}/openssl/OpenSSL-for-iPhone)
	set(OPENSSL_SSL_LIBRARY ${OPENSSL_BUILD_WORKING_DIR}/lib/libssl.a)
	set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_BUILD_WORKING_DIR}/lib/libcrypto.a)
	set(OPENSSL_INCLUDE_DIR ${OPENSSL_BUILD_WORKING_DIR}/include)
else()
	set(OPENSSL_BUILD_COMMENT "Building openssl...")
	set(OPENSSL_SELECT_VERSION cd ${OPENSSL_ROOT_DIR}/openssl && git reset --hard OpenSSL_1_1_0h || echo Never mind)
	set(
		OPENSSL_BUILD_COMMAND
		./build.sh
		--verbose
		--build-dir=${OPENSSL_BUILD_DIR}
		--prefix=${OPENSSL_INSTALL_DIR}
		)
	set(OPENSSL_SSL_LIBRARY ${OPENSSL_INSTALL_DIR}/lib/libssl.a)
	set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_INSTALL_DIR}/lib/libcrypto.a)
	set(OPENSSL_INCLUDE_DIR ${OPENSSL_INSTALL_DIR}/include)
endif()

add_custom_command(
	COMMENT ${OPENSSL_BUILD_COMMENT}
	OUTPUT ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}
	COMMAND [ -f ./openssl/Makefile ] && make distclean -C openssl || echo Never mind
	COMMAND ${OPENSSL_SELECT_VERSION}
	COMMAND ${OPENSSL_BUILD_COMMAND}
	WORKING_DIRECTORY ${OPENSSL_BUILD_WORKING_DIR}
)

add_custom_target(build_openssl DEPENDS ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})

add_library(ssl STATIC IMPORTED GLOBAL)
add_library(crypto STATIC IMPORTED GLOBAL)

set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${OPENSSL_SSL_LIBRARY})
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${OPENSSL_CRYPTO_LIBRARY})

add_dependencies(ssl build_openssl)
add_dependencies(crypto build_openssl)

unset(OpenSSL_LIBRARIES CACHE)
set(
	OpenSSL_LIBRARIES
	crypto ssl
	CACHE INTERNAL "OpenSSL libraries" FORCE
)

set(
	ThirdParty_OPENSSL_INC_DIR
	${OPENSSL_INCLUDE_DIR}
	CACHE INTERNAL "openssl include directory" FORCE
)

