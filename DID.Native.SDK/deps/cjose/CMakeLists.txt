project(cjose)

include(ExternalProject)
include(ProjectDefaults)
include(ExternalConfigureArgs)

set(CONFIGURE_ARGS_INIT
    ${CONFIGURE_ARGS_INIT}
    "LIBS=-lpthread")

set(CONFIGURE_CMD
    aclocal &&
    autoconf &&
    autoreconf -if &&
    ./configure)

set(CONFIGURE_ARGS
    "--prefix=${PROJECT_INT_DIST_DIR}"
    "--with-openssl=${PROJECT_INT_DIST_DIR}"
    "--with-jansson=${PROJECT_INT_DIST_DIR}"
    "--enable-shared=no"
    "--disable-shared"
    "--enable-static=yes"
    ${CONFIGURE_ARGS_INIT})

set(BUILD_CMD "make")

set(INSTALL_CMD "make")
set(INSTALL_ARGS "install")

ExternalProject_Add(
    libcjose

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/cisco/cjose/archive/0.6.1.tar.gz"
    URL_HASH SHA256=208eaa0fa616b44a71d8aa155c40b14c7c9d0fa2bb91d1408824520d2fc1b4dd
    DOWNLOAD_NAME "cjose-0.6.1.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    BUILD_IN_SOURCE 1

    CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
    INSTALL_COMMAND ${INSTALL_CMD} ${INSTALL_ARGS}
)
