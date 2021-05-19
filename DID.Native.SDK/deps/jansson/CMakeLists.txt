project(jansson)

include(ExternalProject)
include(ProjectDefaults)
include(ExternalConfigureArgs)

set(CONFIGURE_CMD
    aclocal &&
    autoconf &&
    autoreconf -if &&
    ./configure)

set(CONFIGURE_ARGS
    "--prefix=${PROJECT_INT_DIST_DIR}"
        "--enable-shared=no"
        "--disable-shared"
        "--enable-static=yes"
    ${CONFIGURE_ARGS_INIT})

    set(BUILD_CMD "make")

    set(INSTALL_CMD "make")
    set(INSTALL_ARGS "install")

ExternalProject_Add(
    libjansson

    PREFIX ${PROJECT_DEPS_BUILD_PREFIX}
    URL "https://github.com/akheron/jansson/archive/v2.13.1.tar.gz"
    URL_HASH SHA256=f22901582138e3203959c9257cf83eba9929ac41d7be4a42557213a22ebcc7a0
    DOWNLOAD_NAME "jansson-2.13.1.tar.gz"
    DOWNLOAD_DIR ${PROJECT_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    BUILD_IN_SOURCE 1

    CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
    INSTALL_COMMAND ${INSTALL_CMD} ${INSTALL_ARGS}
)
