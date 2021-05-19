project(libcurl)

include(ProjectDefaults)
include(ExternalProject)
include(ExternalConfigureArgs)

#    "--with-zlib=${PROJECT_INT_DIST_DIR}"
#    "--with-ssl=${PROJECT_INT_DIST_DIR}/../deps/libspvsdk/external/src/libspvsdk-build/intermediates"

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

ExternalProject_Add(
    libcurl

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://curl.haxx.se/download/curl-7.66.0.tar.gz"
    URL_HASH SHA256=d0393da38ac74ffac67313072d7fe75b1fa1010eb5987f63f349b024a36b7ffb
    DOWNLOAD_NAME "curl-7.66.0.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1
    LOG_DOWNLOAD FALSE

    BUILD_IN_SOURCE 1

    CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
    INSTALL_COMMAND ${INSTALL_CMD}
)
