project(CUnit)

include(CarrierDefaults)
include(ExternalProject)

if(WIN32)
    set_win_build_options(WIN_BUILD_OPTIONS " - Static Lib")

    set(CONFIGURE_CMD "echo")
    set(CONFIGURE_ARGS "Done")

    set(BUILD_CMD "msbuild")
    set(BUILD_ARGS ${WIN_BUILD_OPTIONS}
        "VC15\\CUnit.sln")

    set(INSTALL_CMD "echo")
    set(INSTALL_ARGS "Done")
else()
    include(ExternalConfigureArgs)

    set(CONFIGURE_CMD
        aclocal &&
        autoconf &&
        autoreconf -if &&
        ./configure)

    set(CONFIGURE_ARGS
        "--prefix=${CARRIER_INT_DIST_DIR}"
        "--enable-shared=no"
        "--disable-shared"
        "--enable-static=yes"
        ${CONFIGURE_ARGS_INIT})

    set(BUILD_CMD "make")

    set(INSTALL_CMD "make")
    set(INSTALL_ARGS "install")
endif()

ExternalProject_Add(
    CUnit

    PREFIX ${CARRIER_DEPS_BUILD_PREFIX}
    URL "https://github.com/jacklicn/CUnit/archive/v2.1-3.tar.gz"
    URL_HASH SHA256=b9ba84b54b916d81608572985d558b23bdcd7739180852273948de5cdc3a2cca
    DOWNLOAD_NAME "CUnit-2.1-3.tar.gz"
    DOWNLOAD_DIR ${CARRIER_DEPS_TARBALL_DIR}
    DOWNLOAD_NO_PROGRESS 1

    BUILD_IN_SOURCE 1

    PATCH_COMMAND ${PATCH_EXE} -s -p1 < ${CMAKE_CURRENT_LIST_DIR}/CUnit.patch

    CONFIGURE_COMMAND ${CONFIGURE_CMD} ${CONFIGURE_ARGS}
    BUILD_COMMAND ${BUILD_CMD} ${BUILD_ARGS}
    INSTALL_COMMAND ${INSTALL_CMD} ${INSTALL_ARGS}
)
