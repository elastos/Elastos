project(libcurl)

include(ProjectDefaults)
include(ExternalProject)

#    "--with-zlib=${PROJECT_INT_DIST_DIR}"
#    "--with-ssl=${PROJECT_INT_DIST_DIR}/../deps/libspvsdk/external/src/libspvsdk-build/intermediates"

if(WIN32)
	set(PATCH_ARGS -s -p1 < "${CMAKE_CURRENT_LIST_DIR}/curl.patch")

	if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
		set(MACHINE x64)
	else()
		set(MACHINE x86)
	endif()

	if("${CMAKE_BUILD_TYPE}" MATCHES "(Debug|RelWithDebInfo)")
		set(DEBUG yes)
		set(BUILD_SUFFIX "_debug")
	else()
		set(DEBUG no)
	endif()

	set(CONFIGURE_CMD "echo")
	set(CONFIGURE_ARGS "Done")
	set(BUILD_CMD cd winbuild && nmake)
	set(BUILD_ARGS
		"/f" "Makefile.vc"
		"mode=static"
		"VC=15"
		#"WITH_PREFIX=${PROJECT_INT_DIST_DIR}"
		"WITH_DEVEL=${PROJECT_INT_DIST_DIR}"
		"WITH_SSL=static"
		"WITH_ZLIB=static"
		"ENABLE_WINSSL=no"
		"ENABLE_SSPI=no"
		"ENABLE_IPV6=no"
		"ENABLE_IDN=no"
		"ENABLE_OPENSSL_AUTO_LOAD_CONFIG=no"
		"DEBUG=${DEBUG}"
		"MACHINE=${MACHINE}")

	set(OUTPUT_PATH builds\\libcurl)
	set(INSTALL_CMD
		xcopy "${OUTPUT_PATH}\\include\\curl" "${PROJECT_INT_DIST_DIR}\\include\\curl" /e /i /h /y &&
		copy "${OUTPUT_PATH}\\lib\\libcurl_a${BUILD_SUFFIX}.lib" "${PROJECT_INT_DIST_DIR}\\lib\\curl.lib" /y)
else()
	include(ExternalConfigureArgs)

	set(PATCH_EXE "echo")
	set(PATCH_ARGS "Done")

	set(CONFIGURE_CMD "./configure")
	set(CONFIGURE_ARGS
		"--prefix=${PROJECT_INT_DIST_DIR}"
		"--with-ssl=${PROJECT_INT_DIST_DIR}"
		"--disable-shared"
		"--enable-static"
		"--without-brotli"
		"--without-ldap-lib"
		"--without-lber-lib"
		"--without-winssl"
		"--without-schannel"
		"--without-darwinssl"
		"--without-gnutls"
		"--without-polarssl"
		"--without-mbedtls"
		"--without-cyassl"
		"--without-wolfssl"
		"--without-libpsl"
		"--without-libmetalink"
		"--without-libssh2"
		"--without-libssh"
		"--without-librtmp"
		"--without-winidn"
		"--without-libidn2"
		"--without-nghttp2"
		"--enable-http"
		"--disable-ftp"
		"--disable-file"
		"--disable-ldap"
		"--disable-ldaps"
		"--disable-rtsp"
		"--disable-proxy"
		"--disable-dict"
		"--disable-telnet"
		"--disable-tftp"
		"--disable-pop3"
		"--disable-imap"
		"--disable-smb"
		"--disable-smtp"
		"--disable-gopher"
		"--disable-manual"
		${CONFIGURE_ARGS_INIT}
		"PKG_CONFIG= "
		"ac_tool_prefix=")

	set(INSTALL_CMD
		make install)
endif()

ExternalProject_Add(
	libcurl

	PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
	URL "https://github.com/curl/curl/releases/download/curl-7_71_1/curl-7.71.1.tar.bz2"
	URL_HASH SHA256=9d52a4d80554f9b0d460ea2be5d7be99897a1a9f681ffafe739169afd6b4f224
	DOWNLOAD_NAME "curl-7.71.1.tar.bz2"
	DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
	DOWNLOAD_NO_PROGRESS 1
	LOG_DOWNLOAD FALSE

	BUILD_IN_SOURCE 1

	PATCH_COMMAND ${PATCH_EXE} ${PATCH_ARGS}
	CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
	BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
	INSTALL_COMMAND ${INSTALL_CMD}
)
