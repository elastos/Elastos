
set(OPENSSL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/openssl)
set(OPENSSL_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl/build)
set(OPENSSL_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl/install)

set(OPENSSL_SSL_LIBRARY ${OPENSSL_INSTALL_DIR}/lib/libssl.a)
set(OPENSSL_CRYPTO_LIBRARY ${OPENSSL_INSTALL_DIR}/lib/libcrypto.a)

set(OPENSSL_PREPARE_ENV chmod a+x ${OPENSSL_ROOT_DIR}/Setenv-android.sh && . ${OPENSSL_ROOT_DIR}/Setenv-android.sh)
set(OPENSSL_CONFIG_COMMAND ${OPENSSL_ROOT_DIR}/openssl/config shared --prefix=${OPENSSL_INSTALL_DIR})
set(OPENSSL_BUILD_COMMAND make all && make install_sw)

if(SPV_FOR_ANDROID)
	set(OPENSSL_SELECT_VERSION cd ${OPENSSL_ROOT_DIR}/openssl && git checkout master || echo Never mind)
	set(OPENSSL_BUILD_COMMAND ${OPENSSL_PREPARE_ENV} && ${OPENSSL_CONFIG_COMMAND} && ${OPENSSL_BUILD_COMMAND})
else()
	set(OPENSSL_SELECT_VERSION cd ${OPENSSL_ROOT_DIR}/openssl && git checkout OpenSSL_1_1_0h || echo Never mind)
	set(OPENSSL_BUILD_COMMAND ${OPENSSL_CONFIG_COMMAND} && ${OPENSSL_BUILD_COMMAND})
endif()

add_custom_command(
	COMMENT "Creating openssl build directory..."
	OUTPUT ${OPENSSL_BUILD_DIR}
	COMMAND mkdir -p ${OPENSSL_BUILD_DIR}
)

add_custom_command(
	COMMENT "Building openssl..."
	OUTPUT ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY}
	COMMAND ${OPENSSL_SELECT_VERSION}
	COMMAND ${OPENSSL_BUILD_COMMAND}
	WORKING_DIRECTORY ${OPENSSL_BUILD_DIR}
	DEPENDS ${OPENSSL_BUILD_DIR}
)

add_custom_target(build_openssl ALL DEPENDS ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})

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
	${OPENSSL_INSTALL_DIR}/include
	CACHE INTERNAL "openssl include directory" FORCE
)

