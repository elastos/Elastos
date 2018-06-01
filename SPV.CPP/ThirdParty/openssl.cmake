
set(
	ThirdParty_OPENSSL_INC_DIR
	${CMAKE_CURRENT_SOURCE_DIR}/openssl/install/include
	CACHE INTERNAL "openssl include directory" FORCE
)

set(OPENSSL_SSL_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/openssl/install/lib/libssl.a)
set(OPENSSL_CRYPTO_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/openssl/install/lib/libcrypto.a)
set(OPENSSL_PREPARE_ENV chmod a+x ../Setenv-android.sh && source ../Setenv-android.sh)
set(OPENSSL_CONFIG_COMMAND ./config shared --openssldir=${CMAKE_CURRENT_SOURCE_DIR}/openssl/install --prefix=${CMAKE_CURRENT_SOURCE_DIR}/openssl/install)
set(OPENSSL_BUILD_COMMAND make all && make install_sw)

if(SPV_FOR_ANDROID)
	set(OPENSSL_SELECT_VERSION git checkout master || echo Never mind)
	set(OPENSSL_BUILD_COMMAND ${OPENSSL_PREPARE_ENV} && ${OPENSSL_CONFIG_COMMAND} && ${OPENSSL_BUILD_COMMAND})
else()
	set(OPENSSL_SELECT_VERSION git checkout OpenSSL_1_1_0h || echo Never mind)
	set(OPENSSL_BUILD_COMMAND ${OPENSSL_CONFIG_COMMAND} && ${OPENSSL_BUILD_COMMAND})
endif()

add_custom_command(
	COMMENT "Building openssl..."
	OUTPUT "${OPENSSL_SSL_LIBRARY}" "${OPENSSL_CRYPTO_LIBRARY}"
	COMMAND ${OPENSSL_SELECT_VERSION}
	COMMAND ${OPENSSL_BUILD_COMMAND}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/openssl/openssl
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/openssl/Setenv-android.sh
)

#add_custom_target(
#	clean_openssl ALL
#	COMMENT "Cleaning openssl..."
#	COMMAND [ -f ${CMAKE_CURRENT_SOURCE_DIR}/openssl/openssl/Makefile ] && make distclean || echo Never mind
#	COMMAND rm -fr ../install
#	COMMAND git checkout master
#	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/openssl/openssl
#)

add_custom_target(
	clean_openssl ALL
	COMMENT "Cleaning openssl..."
	COMMAND [ -f ${CMAKE_CURRENT_SOURCE_DIR}/openssl/openssl/Makefile ] && make distclean || echo Never mind
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/openssl/openssl
)

add_custom_target(build_openssl ALL DEPENDS clean_openssl ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})

add_library(ssl STATIC IMPORTED GLOBAL)
add_library(crypto STATIC IMPORTED GLOBAL)

set_target_properties(ssl PROPERTIES IMPORTED_LOCATION ${OPENSSL_SSL_LIBRARY})
set_target_properties(crypto PROPERTIES IMPORTED_LOCATION ${OPENSSL_CRYPTO_LIBRARY})

add_dependencies(ssl build_openssl)
add_dependencies(crypto build_openssl)
