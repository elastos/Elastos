project(curl)

include(CarrierDefaults)
include(ExternalProject)

if(WIN32)
    set(PATCH_ARGS -s -p1 < "${CMAKE_CURRENT_LIST_DIR}/curl.patch")

    set(CONFIGURE_CMD "echo")
    set(CONFIGURE_ARGS "Done")

    set(BUILD_CMD cd winbuild && nmake)
    set(BUILD_ARGS
        -f Makefile.vc
        MODE=static
        "WITH_DEVEL=${CARRIER_INT_DIST_DIR}"
        WITH_SSL=static
        WITH_ZLIB=static
        ENABLE_WINSSL=no
        ENABLE_IDN=no)

    set(OUTPUT_PATH builds\\libcurl)
    set(INSTALL_CMD
        xcopy "${OUTPUT_PATH}\\include\\curl" "${CARRIER_INT_DIST_DIR}\\include\\curl" /e /i /h /y &&
        copy "${OUTPUT_PATH}\\lib\\libcurl_a.lib" "${CARRIER_INT_DIST_DIR}\\lib\\curl.lib" /y)
else()
    include(ExternalConfigureArgs)

    set(PATCH_EXE "echo")
    set(PATCH_ARGS "Done")

    set(CONFIGURE_CMD "./configure")
    set(CONFIGURE_ARGS
        "--prefix=${CARRIER_INT_DIST_DIR}"
        "--with-ssl=${CARRIER_INT_DIST_DIR}"
        "--with-zlib=${CARRIER_INT_DIST_DIR}"
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
        ${CONFIGURE_ARGS_INIT})

    set(INSTALL_CMD
        make install)
endif()

ExternalProject_Add(
    curl

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://github.com/curl/curl/releases/download/curl-7_64_0/curl-7.64.0.tar.bz2"
    URL_HASH SHA256=d573ba1c2d1cf9d8533fadcce480d778417964e8d04ccddcc76e591d544cf2eb
    DOWNLOAD_NAME "curl-7.64.0.tar.bz2"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1
    LOG_DOWNLOAD FALSE

    BUILD_IN_SOURCE 1

    PATCH_COMMAND ${PATCH_EXE} ${PATCH_ARGS}
    CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
    INSTALL_COMMAND ${INSTALL_CMD}
)

set(CURL_DEPS
    curl
    ssl
    crypto
    z)

if(WIN32)
    set(CURL_DEPS ${CURL_DEPS} Ws2_32 Iphlpapi)
endif()

if(NOT ANDROID)
    set(CURL_DEPS ${CURL_DEPS} pthread)
endif()

add_library(libcurl INTERFACE)
target_link_libraries(libcurl INTERFACE ${CURL_DEPS})
if(WIN32)
    target_compile_definitions(libcurl INTERFACE CURL_STATICLIB)
endif()

if(ENABLE_STATIC)
    export_static_library(curl)
endif()
